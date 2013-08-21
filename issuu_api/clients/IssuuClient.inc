<?php
/**
 * @file
 * Defines the IssuuClient and IssuuRequestParams classes for use in the
 * issuu_api module.
 */

/**
 * Base type for all Issuu related service interaction.
 * @abstract
 */
abstract class IssuuClient {

  private $_http_client;
  private $_settings;

  //----------------------------------------------------------------------------
  // Constructor
  //----------------------------------------------------------------------------

  /**
   * Create a new instance of IssuuClient.
   */
  public function __construct(array $settings = NULL) {
    // Initialize object.
    $this->initialize();
    // Save the settings.
    $this->_settings = isset($settings) ? $settings : array();
  }

  //----------------------------------------------------------------------------
  // Initialization methods.
  //----------------------------------------------------------------------------

  /**
   * Initialize object attributes.
   */
  protected function initialize() {
    // Initialize the HTTP client.
    $this->_http_client = $this->createHttpClient();
  }

  /**
   * Create a HttpClient object.
   * @return HttpClient Returns a HttpClient object.
   */
  private function createHttpClient() {
    $http_client_formatter = $this->createHttpClientFormatter();
    $http_client_delegate = $this->createHttpClientDelegate();
    return http_client(NULL, $http_client_formatter, array($this, 'alterHttpClientRequest'), $http_client_delegate);
  }

  /**
   * Create a HttpClientFormatter.
   * @return HttpClientFormatter Returns a HttpClientFormatter object.
   */
  protected function createHttpClientFormatter() {
    return new IssuuApiFormatter();
  }

  /**
   * Create a HttpClientDelegate.
   * @return HttpClientDelegate Returns a HttpClientDelegate object.
   */
  protected function createHttpClientDelegate() {
    // TODO: Change following line to "return new HttpClientCurlDelegate();" if
    // the http_client issue gets approved. (http://drupal.org/node/1339912).
    return new IssuuApiCurlDelegate();
  }

  //----------------------------------------------------------------------------
  // Request manipulation functions.
  //----------------------------------------------------------------------------

  /**
   * Allow the IssuuApiClient to alter the specified HttpClientRequest object.
   * @param HttpClientRequest $request A HttpClientRequest object.
   */
  public function alterHttpClientRequest(HttpClientRequest $request) {
    // TODO: Add proxy server support using $this->getSetting('proxy');
  }

  //----------------------------------------------------------------------------
  // Request execution functions.
  //----------------------------------------------------------------------------

  /**
   * Execute HTTP GET request to the specified URL.
   * @param $url The request URL.
   * @param IssuuRequestParams $parameters The query parameters.
   * @return Returns the response from the specified URL if successful,
   * otherwise an error code is returned.
   */
  protected final function get($url, IssuuRequestParams $parameters) {
    $result = ISSUU_API_ERROR_GENERAL_FAULT;
    // Validate function arguments.
    if (isset($url) && valid_url($url)) {
      try {
        // Perform get request and retrieve response.
        $response = $this->_http_client->get($url, $parameters->getRequestParameters());
        // Retrieve and return the Issuu API response.
        $result = $this->getIssuuResponse($response);
      }
      catch (Exception $ex) {
        // Log exception to watchdog.
        watchdog_exception('issuu_api', $ex);
        // Return the exception code.
        $result = $ex->getCode();
      }
    }
    return $result;
  }

  /**
   * Execute HTTP POST request to the specified URL.
   * @param $url The request URL.
   * @param IssuuRequestParams $parameters The query parameters.
   * @return Returns the response from the specified URL if successful,
   * otherwise an error code is returned.
   */
  protected final function post($url, IssuuRequestParams $parameters, $multipart = FALSE) {
    $result = ISSUU_API_ERROR_GENERAL_FAULT;
    // Validate function arguments.
    if (isset($url) && valid_url($url, TRUE)) {
      try {
        // Perform post request and retrieve response.
        $response = $this->_http_client->post($url, array('multipart' => $multipart, 'parameters' => $parameters));
        // Retrieve and return the Issuu API response.
        return $this->getIssuuResponse($response);
      }
      catch (Exception $ex) {
        // Log exception to watchdog.
        watchdog_exception('issuu_api', $ex);
        // Return the exception code.
        return $ex->getCode();
      }
    }
    return $result;
  }

  //----------------------------------------------------------------------------
  // Response processing functions.
  //----------------------------------------------------------------------------

  /**
   * Process the Issuu response.
   * @param $response The HTTP response from Issuu.
   * @return Returns the usable response data.
   * @throws InvalidArgumentException
   */
  protected function getIssuuResponse($response) {
    // Validate function argument.
    if (!empty($response) && is_array($response)) {
      return $response;
    }
    throw new InvalidArgumentException('Issuu API repsonse was malformed.', ISSUU_API_ERROR_GENERAL_FAULT);
  }

  //----------------------------------------------------------------------------
  // Settings functions.
  //----------------------------------------------------------------------------

  /**
   * Get the specified setting.
   * @param $name The setting name.
   * @param $default_value The default value if the setting is not set.
   * @return Returns the setting for the specified setting name.
   */
  public function getSetting($name, $default_value = NULL) {
    if (isset($name) && is_string($name) && isset($this->_settings[$name])) {
      return $this->_settings[$name];
    }
    return $default_value;
  }

  /**
   * Set the specified setting.
   * @param $name The setting name.
   * @param $value The setting value.
   */
  public function setSetting($name, $value) {
    if (isset($name) && is_string($name)) {
      $this->_settings[$name] = $value;
    }
  }

  /**
   * Remove the specified setting.
   * @param $name The setting name.
   */
  public function removeSetting($name) {
    if (isset($name) && is_string($name)) {
      unset($this->_settings[$name]);
    }
  }

}

//------------------------------------------------------------------------------
// IssuuRequestParams class definition.
//------------------------------------------------------------------------------

/**
 * Provides the functionality to handle Issuu related request parameters.
 */
class IssuuRequestParams {

  private $_request_params;

  //----------------------------------------------------------------------------
  // Constructor.
  //----------------------------------------------------------------------------

  /**
   * Create a new instance of IssuuRequestParams.
   */
  public function __constructor(array $params = NULL) {
    // Initialize the request params member.
    $this->_request_params = array();
    // Add the $params to the request parameters if set.
    if (isset($params)) {
      // Iterate through the $params.
      foreach ($params as $name => $value) {
        // We do not assign the $params to the request param directly but add
        // them using addParam function. This way we prevent incorrect data
        // being added to the request parameters.
        $this->addParam($name, $value);
      }
    }
  }

  //----------------------------------------------------------------------------
  // Request parameter functions.
  //----------------------------------------------------------------------------

  /**
   * Add the specified request parameter with the corresponding value.
   * @param $name The request parameter name.
   * @param $value The request parameter value. Arrays are converted to a comma-
   * seperated list.
   */
  public final function addParam($name, $value) {
    // Only add request parameters with a valid name and value.
    if (isset($name) && is_string($name) && isset($value)) {
      // Implode $value if it's an array.
      if (is_array($value)) {
        $value = implode(',', $value);
      }
      // Add the request parameter.
      $this->_request_params[$name] = $value;
    }
  }

  /**
   * Remove the specified request parameter.
   * @param $name The request parameter name.
   */
  public final function removeParam($name) {
    // Only remove the request parameter if it has a valid name.
    if (isset($name) && is_string($name)) {
      unset($this->_request_params[$name]);
    }
  }

  //----------------------------------------------------------------------------
  // Type specific request parameters.
  //----------------------------------------------------------------------------

  /**
   * Add an absolute URL request parameter.
   * @param $name The request parameter name.
   * @param $value The request parameter value.
   * @throws InvalidArgumentException
   */
  public final function addUrlParam($name, $value) {
    if (isset($value)) {
      // Validate request parameter value.
      if (!is_string($value) || !valid_url($value)) {
        throw new InvalidArgumentException("Parameter \"{$name}\" must contain an absolute URL value.", ISSUU_API_ERROR_INVALID_FIELD_FORMAT);
      }
      // Add request parameter.
      $this->addParam($name, $value);
    }
  }

  /**
   * Add a boolean string request parameter.
   * @param $name The request parameter name.
   * @param $value The request parameter value.
   * @throws InvalidArgumentException
   */
  public final function addBooleanStringParam($name, $value) {
    if (isset($value)) {
      // Validate request parameter value.
      if (is_bool($value)) {
        $value = ($value ? 'true' : 'false');
      }
      else {
        throw new InvalidArgumentException("Parameter \"{$name}\" must contain a boolean value.", ISSUU_API_ERROR_INVALID_FIELD_FORMAT);
      }
      // Add request parameter.
      $this->addParam($name, $value);
    }
  }

  /**
   * Add a boolean integer request parameter.
   * @param $name The request parameter name.
   * @param $value The request parameter value.
   * @throws InvalidArgumentException
   */
  public final function addBooleanIntegerParam($name, $value) {
    if (isset($value)) {
      // Validate request parameter value.
      if (is_bool($value)) {
        $value = (int) $value;
      }
      else {
        throw new InvalidArgumentException("Parameter \"{$name}\" must contain a boolean value.", ISSUU_API_ERROR_INVALID_FIELD_FORMAT);
      }
      // Add request parameter.
      $this->addParam($name, $value);
    }
  }

  /**
   * Add an integer request parameter.
   * @param $name The request parameter name.
   * @param $value The request parameter value.
   * @throws InvalidArgumentException
   */
  public final function addIntegerParam($name, $value) {
    if (isset($value)) {
      // Validate request parameter value.
      if (!is_numeric($value)) {
        throw new InvalidArgumentException("Parameter \"{$name}\" must contain a numeric value.", ISSUU_API_ERROR_INVALID_FIELD_FORMAT);
      }
      // Add request parameter.
      $this->addParam($name, $value);
    }
  }

  /**
   * Add a string request parameter.
   * @param $name The request parameter name.
   * @param $value The request parameter value.
   * @throws InvalidArgumentException
   */
  public final function addStringParam($name, $value) {
    if (isset($value)) {
      $this->addStringListParam($name, array($value));
    }
  }

  /**
   * Add an array of string request parameters.
   * @param $name The request parameter name.
   * @param $value The request parameter value.
   * @throws InvalidArgumentException
   */
  public final function addStringListParam($name, $value) {
    if (!empty($value) && is_array($value)) {
      // Iterate throught the values.
      foreach ($value as $param_value) {
        // Validate request parameter value.
        if (!is_string($param_value)) {
          throw new InvalidArgumentException("Parameter \"{$name}\" must contain a string value.", ISSUU_API_ERROR_INVALID_FIELD_FORMAT);
        }
      }
      // Add request parameter.
      $this->addParam($name, $value);
    }
  }

  /**
   * Add a timestamp request parameter.
   * @param $name The request parameter name.
   * @param $value The request parameter value.
   * @throws InvalidArgumentException
   */
  public final function addTimestampParam($name, $value) {
    if (isset($value)) {
      // Validate request parameter value.
      if (!is_numeric($value)) {
        throw new InvalidArgumentException("Parameter \"{$name}\" must contain a timestamp value.", ISSUU_API_ERROR_INVALID_FIELD_FORMAT);
      }
      // Add request parameter.
      $this->addParam($name, date('Y-m-d\TH:i:s\Z', $value));
    }
  }

  /**
   * Add a file request parameter.
   * @param $name The request parameter name.
   * @param $value The request parameter value.
   * @throws InvalidArgumentException
   */
  public final function addFileParam($name, $value) {
    if (isset($value)) {
      // Validate request parameter value.
      if (!is_string($value) || !is_file($value)) {
        throw new InvalidArgumentException("Parameter \"{$name}\" must contain a valid file location.", ISSUU_API_ERROR_INVALID_FIELD_FORMAT);
      }
      // Add request parameter.
      $this->addParam($name, "@{$value}");
    }
  }

  /**
   * Add a enumeration request parameter.
   * @param $name The request parameter name.
   * @param $value The request parameter value.
   * @param $allowed_values The allowed enumeration values.
   * @throws InvalidArgumentException
   */
  public final function addEnumParam($name, $value, array $allowed_values) {
    if (isset($value)) {
      $this->addEnumListParam($name, array($value), $allowed_values);
    }
  }

  /**
   * Add a enumeration request parameter.
   * @param $name The request parameter name.
   * @param $value The request parameter value.
   * @param $allowed_values The allowed enumeration values.
   * @throws InvalidArgumentException
   */
  public final function addEnumListParam($name, $value, array $allowed_values) {
    if (!empty($value) && is_array($value)) {
      // Iterate throught the values.
      foreach ($value as $param_value) {
        // Validate request parameter value.
        if (!isset($param_value) || !isset($allowed_values[$param_value])) {
          throw new InvalidArgumentException("Parameter \"{$name}\" must contain a value defined by the enumeration.", ISSUU_API_ERROR_INVALID_FIELD_FORMAT);
        }
      }
      // Add request parameter.
      $this->addParam($name, $value);
    }
  }

  //----------------------------------------------------------------------------
  // Helper functions.
  //----------------------------------------------------------------------------

  /**
   * Get the request parameters.
   * @return Returns an associative array containing the request parameters,
   * keyed by the request parameter name.
   */
  public final function getRequestParameters() {
    return (array) $this->_request_params;
  }

  /**
   * Get an indication whether the specified request parameter is present.
   * @param $name The request parameter name.
   * @return Returns TRUE if the request parameter is present, otherwise FALSE.
   */
  public final function hasRequestParameter($name) {
    if (isset($name) && is_string($name)) {
      return isset($this->_request_params[$name]);
    }
    return FALSE;
  }

}