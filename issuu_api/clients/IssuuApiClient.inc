<?php
/**
 * @file
 * Defines the IssuuApiClient class for use in the issuu_api module.
 */

/**
 * Provides the functionality to interact with the Issuu API.
 */
class IssuuApiClient extends IssuuClient {

  const API_ENDPOINT = 'http://api.issuu.com/1_0';
  const UPLOAD_ENDPOINT = 'http://upload.issuu.com/1_0';

  //----------------------------------------------------------------------------
  // Constructor
  //----------------------------------------------------------------------------

  /**
   * Create a new instance of IssuuApiClient.
   */
  public function __construct(array $settings) {
    parent::__construct($settings);
  }

  //----------------------------------------------------------------------------
  // Request manipulation functions.
  //----------------------------------------------------------------------------

  /**
   * Allow the IssuuApiClient to alter the specified HttpClientRequest object.
   * @param HttpClientRequest $request A HttpClientRequest object.
   */
  public function alterHttpClientRequest(HttpClientRequest $request) {
    // Allow the parent to alter the $request.
    parent::alterHttpClientRequest($request);
    // Override the CURLOPT_HTTPHEADER option:
    if (($curlopts = &$request->options['curlopts']) === NULL) {
      $curlopts = array();
    }
    // Override expect header (do not remove!)
    // See: http://the-stickman.com/web-development/php-and-curl-disabling-100-continue-header/
    $curlopts[CURLOPT_HTTPHEADER] = array('Expect:');
  }

  //----------------------------------------------------------------------------
  // General API interaction functions.
  //----------------------------------------------------------------------------

  /**
   * Perform the specified action against the given endpoint.
   * @param $endpoint The endpoint to interact with.
   * @param $action The action to perform.
   * @param IssuuRequestParams $parameters The request parameters.
   * @return Returns the Issuu response for the specified action if successful,
   * otherwise an error code is returned.
   */
  protected function action($endpoint, $action, IssuuRequestParams $parameters, $multipart = FALSE) {
    if (isset($endpoint) && valid_url($endpoint) && isset($action) && is_string($action)) {
      // Add Issuu API action.
      $parameters->addParam('action', $action);
      // This module only support the json format.
      $parameters->addParam('format', 'json');
      // Set the API key.
      $parameters->addParam('apiKey', $this->getSetting('api_key', ''));
      // Retrieve the request parameters.
      $request_params = $parameters->getRequestParameters();
      // Perform the signing algorithm:
      // (See http://issuu.com/services/api/signingrequests.html for more
      // information)
      // 1. Sort request parameters alphabetically (e.g. foo=1, bar=2, baz=3 sorts
      // to bar=2, baz=3, foo=1)
      ksort($request_params);
      // 2. Concatenate in order your API secret key and request name-value pairs
      // (e.g. SECRETbar2baz3foo1)
      $signature_concat = $this->getSetting('secret_key', '');
      foreach ($request_params as $name => $value) {
        // Do not include the file request parameter in the signature calculation.
        if ($name != 'file') {
          // Concat to signature build.
          $signature_concat .= "{$name}{$value}";
        }
      }
      // 3. Calculate the signature as the MD5 hash of this string
      $parameters->addParam('signature', md5($signature_concat));
      // Perform action agains the given $endpoint.
      return $this->post($endpoint, $parameters, $multipart);
    }
    return ISSUU_API_ERROR_GENERAL_FAULT;
  }

  //----------------------------------------------------------------------------
  // Bookmark API interaction functions.
  //----------------------------------------------------------------------------

  /**
   * Create a bookmark of any Issuu document. The bookmark can be optionally
   * added to one or more folders.
   * @param IssuuRequestParams $parameters The request parameters.
   * @return Returns the added bookmark if successful, otherwise an error code
   * is returned.
   */
  public function issuu_bookmark_add(IssuuRequestParams $parameters) {
    $response = ISSUU_API_ERROR_MISSING_REQUIRED_FIELD;
    // Validate required fields.
    if ($parameters->hasRequestParameter('documentUsername') &&
        $parameters->hasRequestParameter('name')) {
      // Perform the action.
      $response = $this->action(self::API_ENDPOINT, 'issuu.bookmark.add', $parameters);
      // Check if the action was successful.
      if (issuu_api_succeeded($response)) {
        $response = $response['bookmark'];
      }
    }
    return $response;
  }

  /**
   * List of all bookmarks belonging to a user profile.
   * @param IssuuRequestParams $parameters The request parameters.
   * @return Returns the bookmark list if successful, otherwise an error code is
   * returned.
   */
  public function issuu_bookmarks_list(IssuuRequestParams $parameters = NULL) {
    // Perform the action.
    $response = $this->action(self::API_ENDPOINT, 'issuu.bookmarks.list', (isset($parameters) ? $parameters : new IssuuRequestParams()));
    // Check if the action was successful.
    if (issuu_api_succeeded($response)) {
      $response = $response['result'];
    }
    return $response;
  }

  /**
   * Update title or description of a bookmark.
   * @param IssuuRequestParams $parameters The request parameters.
   * @return Returns the updated bookmark if successful, otherwise an error code
   * is returned.
   */
  public function issuu_bookmark_update(IssuuRequestParams $parameters) {
    $response = ISSUU_API_ERROR_MISSING_REQUIRED_FIELD;
    // Validate required fields.
    if ($parameters->hasRequestParameter('bookmarkId')) {
      // Perform the action.
      $response = $this->action(self::API_ENDPOINT, 'issuu.bookmark.update', $parameters);
      // Check if the action was successful.
      if (issuu_api_succeeded($response)) {
        $response = $response['bookmark'];
      }
    }
    return $response;
  }

  /**
   * Delete one or more bookmarks.
   * @param IssuuRequestParams $parameters The request parameters.
   * @return Returns TRUE if the bookmarks were deleted, otherwise an error code
   * is returned.
   */
  public function issuu_bookmark_delete(IssuuRequestParams $parameters) {
    $response = ISSUU_API_ERROR_MISSING_REQUIRED_FIELD;
    // Validate required fields.
    if ($parameters->hasRequestParameter('bookmarkIds')) {
      // Perform the action.
      $response = $this->action(self::API_ENDPOINT, 'issuu.bookmark.delete', $parameters);
    }
    return $response;
  }

  //----------------------------------------------------------------------------
  // Document API interaction functions.
  //----------------------------------------------------------------------------

  /**
   * Upload a document to a user's profile. The document can be automatically
   * added to a folder once it has been converted.
   * @param IssuuRequestParams $parameters The request parameters.
   * @return Returns the uploaded Issuu document information if successful,
   * otherwise an error code is returned.
   */
  public function issuu_document_upload(IssuuRequestParams $parameters) {
    $response = ISSUU_API_ERROR_MISSING_REQUIRED_FIELD;
    // Validate required fields.
    if ($parameters->hasRequestParameter('file')) {
      // Perform the action.
      $response = $this->action(self::UPLOAD_ENDPOINT, 'issuu.document.upload', $parameters, TRUE);
      // Check if the action was successful.
      if (issuu_api_succeeded($response)) {
        $response = $response['document'];
      }
    }
    return $response;
  }

  /**
   * Add a document to a user's profile by specifying its location on the web.
   * The document can be automatically added to a folder once it has been
   * converted.
   * @param IssuuRequestParams $parameters The request parameters.
   * @return Returns the uploaded Issuu document information if successful,
   * otherwise an error code is returned.
   */
  public function issuu_document_url_upload(IssuuRequestParams $parameters) {
    $response = ISSUU_API_ERROR_MISSING_REQUIRED_FIELD;
    // Validate required fields.
    if ($parameters->hasRequestParameter('slurpUrl')) {
      // Perform the action.
      $response = $this->action(self::UPLOAD_ENDPOINT, 'issuu.document.url_upload', $parameters);
      // Check if the action was successful.
      if (issuu_api_succeeded($response)) {
        $response = $response['document'];
      }
    }
    return $response;
  }

  /**
   * List of documents belonging to a user profile and which satisfies certain
   * criteria.
   * @param IssuuRequestParams $parameters The request parameters.
   * @return Returns the Issuu documents list if successful, otherwise an error
   * code is returned.
   */
  public function issuu_documents_list(IssuuRequestParams $parameters = NULL) {
    // Perform the action.
    $response = $this->action(self::API_ENDPOINT, 'issuu.documents.list', (isset($parameters) ? $parameters : new IssuuRequestParams()));
    // Check if the action was successful.
    if (issuu_api_succeeded($response)) {
      $response = $response['result'];
    }
    return $response;
  }

  /**
   * Update a document with the given information.
   * @param IssuuRequestParams $parameters The request parameters.
   * @return Returns the updated Issuu document if successful, otherwise an
   * error code is returned.
   */
  public function issuu_document_update(IssuuRequestParams $parameters) {
    $response = ISSUU_API_ERROR_MISSING_REQUIRED_FIELD;
    // Validate required fields.
    if ($parameters->hasRequestParameter('name')) {
      // Perform the action.
      $response = $this->action(self::API_ENDPOINT, 'issuu.document.update', $parameters);
      // Check if the action was successful.
      if (issuu_api_succeeded($response)) {
        $response = $response['document'];
      }
    }
    return $response;
  }

  /**
   * Delete one or more documents including all comments and ratings.
   * @param IssuuRequestParams $parameters The request parameters.
   * @return Returns TRUE if the documents were deleted, otherwise an error code
   * is returned.
   */
  public function issuu_document_delete(IssuuRequestParams $parameters) {
    $response = ISSUU_API_ERROR_MISSING_REQUIRED_FIELD;
    // Validate required fields.
    if ($parameters->hasRequestParameter('names')) {
      // Perform the action.
      $response = $this->action(self::API_ENDPOINT, 'issuu.document.delete', $parameters);
    }
    return $response;
  }

  //----------------------------------------------------------------------------
  // Folder API interaction functions.
  //----------------------------------------------------------------------------

  /**
   * Create an empty folder. The folder can later be filled with bookmarks of
   * any document on Issuu.
   * @param IssuuRequestParams $parameters The request parameters.
   * @return Returns the added folder if successful, otherwise an error code
   * is returned.
   */
  public function issuu_folder_add(IssuuRequestParams $parameters) {
    $response = ISSUU_API_ERROR_MISSING_REQUIRED_FIELD;
    // Validate required fields.
    if ($parameters->hasRequestParameter('folderName')) {
      // Perform the action.
      $response = $this->action(self::API_ENDPOINT, 'issuu.folder.add', $parameters);
      // Check if the action was successful.
      if (issuu_api_succeeded($response)) {
        $response = $response['folder'];
      }
    }
    return $response;
  }

  /**
   * List all folders belonging to a user profile.
   * @param IssuuRequestParams $parameters The request parameters.
   * @return Returns the folder list if successful, otherwise an error code is
   * returned.
   */
  public function issuu_folders_list(IssuuRequestParams $parameters = NULL) {
    // Perform the action.
    $response = $this->action(self::API_ENDPOINT, 'issuu.folders.list', (isset($parameters) ? $parameters : new IssuuRequestParams()));
    // Check if the action was successful.
    if (issuu_api_succeeded($response)) {
      $response = $response['result'];
    }
    return $response;
  }

  /**
   * Update folder name or description.
   * @param IssuuRequestParams $parameters The request parameters.
   * @return Returns the updated folder if successful, otherwise an error code
   * is returned.
   */
  public function issuu_folder_update(IssuuRequestParams $parameters) {
    $response = ISSUU_API_ERROR_MISSING_REQUIRED_FIELD;
    // Validate required fields.
    if ($parameters->hasRequestParameter('folderId')) {
      // Perform the action.
      $response = $this->action(self::API_ENDPOINT, 'issuu.folder.update', $parameters);
      // Check if the action was successful.
      if (issuu_api_succeeded($response)) {
        $response = $response['folder'];
      }
    }
    return $response;
  }

  /**
   * Delete one or more folders.
   * @param IssuuRequestParams $parameters The request parameters.
   * @return Returns TRUE if the folders were deleted, otherwise an error code
   * is returned.
   */
  public function issuu_folder_delete(IssuuRequestParams $parameters) {
    $response = ISSUU_API_ERROR_MISSING_REQUIRED_FIELD;
    // Validate required fields.
    if ($parameters->hasRequestParameter('folderIds')) {
      // Perform the action.
      $response = $this->action(self::API_ENDPOINT, 'issuu.folder.delete', $parameters);
    }
    return $response;
  }

  //----------------------------------------------------------------------------
  // Response processing functions.
  //----------------------------------------------------------------------------

  /**
   * Process the Issuu response.
   * @param $response The HTTP response from Issuu.
   * @return Returns the usable response data.
   * @throws InvalidArgumentException
   * @throws IssuuApiResponseException
   */
  protected function getIssuuResponse($response) {
    // Validate function argument.
    if (!empty($response) && is_array($response) && isset($response['rsp'])) {
      // Retrieve Issuu response.
      $issuu_response = $response['rsp'];
      // Evalutate the Issuu response state.
      switch ($issuu_response['stat']) {
        case 'ok':
          // Return the content of the Issuu response.
          return isset($issuu_response['_content']) ? $issuu_response['_content'] : TRUE;
        case 'fail':
          // Retrieve the error content of the Issuu response.
          $error_content = $issuu_response['_content']['error'];
          // Retrieve error code.
          $code = isset($error_content['code']) ? (int) $error_content['code'] : ISSUU_API_ERROR_GENERAL_FAULT;
          // Construct watchdog variables.
          $watchdog_variables = array(
            '@code' => $code,
            '@message' => isset($error_content['message']) ? $error_content['message'] : '',
          );
          // Log the error to the watchdog.
          if (isset($error_content['field'])) {
            // Log field related Issuu error.
            $watchdog_variables['@field'] = $error_content['field'];
            watchdog('issuu_api', 'EC-@code on field "@field": @message', $watchdog_variables, WATCHDOG_ERROR);
          }
          else {
            // Log non field related Issuu error.
            watchdog('issuu_api', 'EC-@code: @message', $watchdog_variables, WATCHDOG_ERROR);
          }
          // Return the error code.
          return $code;
      }
    }
    throw new InvalidArgumentException('Issuu API repsonse was malformed.', ISSUU_API_ERROR_GENERAL_FAULT);
  }

}