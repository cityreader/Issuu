<?php

/**
 * @file
 * Define how to interact with issuu.com API
 */

define('ISSUU_URI', 'http://api.issuu.com/1_0');
define('ISSUU_URI_UPLOAD', 'http://upload.issuu.com/1_0');
define('ISSUU_URI_IMAGE', 'http://image.issuu.com/');
define('ISSUU_DOCUMENTS_LIST', 'issuu.documents.list');
define('ISSUU_DOCUMENT_UPLOAD', 'issuu.document.upload');
define('ISSUU_VIEWER', 'http://static.issuu.com/webembed/viewers/style1/v1/IssuuViewer.swf');
define('ISSUU_LAYOUT', 'http://skin.issuu.com/v/light/layout.xml');

/**
 * Return possible document types.
 *
 * @see http://issuu.com/services/api/documenttypes.html
 */
function issuu_document_types() {
  return array(
    '000000' => 'Unknown',
    '001000' => 'Article or Essay',
    '002000' => 'Book',
    '003000' => 'Catalog',
    '004000' => 'Journal',
    '005000' => 'Magazine',
    '006000' => 'Manual or Resource',
    '007000' => 'Newspaper',
    '008000' => 'Paper',
    '009000' => 'Photo album',
    '010000' => 'Portfolio',
    '011000' => 'Presentation',
    '012000' => 'Report',
    '013000' => 'Other',
    );
}

/**
 * Return possible document categories.
 *
 * @see http://issuu.com/services/api/documentcategories.html
 */
function issuu_document_categories() {
  return array(
    '000000' => 'Unknown',
    '001000' => 'Auto & Vehicles',
    '002000' => 'Business & Marketing',
    '003000' => 'Creative',
    '004000' => 'Film & Music',
    '005000' => 'Fun & Entertainment',
    '006000' => 'Hobby & Home',
    '007000' => 'Knowledge & Resources',
    '008000' => 'Nature & Animals',
    '009000' => 'News & Politics',
    '010000' => 'Nonprofits & Activism',
    '011000' => 'Religon & Philosophy',
    '012000' => 'Sports',
    '013000' => 'Technology & Internet',
    '014000' => 'Travel & Events',
    '015000' => 'Weird & Bizarre',
    '016000' => 'Other'
    );
}

/**
 * Return possible document categories.
 *
 * @see http://issuu.com/services/api/languagecodes.html
 * issuu uses the same standard than drupal(ISO 639-1) so modify a little _locale_prepare_predefined_list() from core
 */
function issuu_get_languages() {
  $predefined = _locale_get_predefined_list();
  foreach ($predefined as $key => $value) {
    // Issuu doesn't support variants (for example pt-br), so skip those.
    if (strlen($key) != 2) {
      unset($predefined[$key]);
      continue;
    }
    // Include native name in output, if possible
    if (count($value) > 1) {
      $tname = t($value[0]);
      $predefined[$key] = ($tname == $value[1]) ? $tname : "$tname ($value[1])";
    }
    else {
      $predefined[$key] = t($value[0]);
    }
  }
  asort($predefined);
  return $predefined;
}

function issuu_api_issuu_documents_list($parameters = NULL) {
  $issuuapi = new IssuuApi();
  return $result = $issuuapi->call(ISSUU_DOCUMENTS_LIST, $parameters);
}

class IssuuApi {
  private $cache = array();
  
  public function __construct() {
    
  }

  public function call($action, $parameters = NULL) {
    $class = str_replace(' ', '', ucwords(strtolower(str_replace('.', ' ', $action))));
    $class = 'IssuuApiMethod_' . $class;

    if (class_exists($class)) {
      return $this->process(new $class(), $parameters);
    }
    return FALSE;
  }

  public function process(IssuuApiMethod $method_obj, $parameters) {
    $parameters = $method_obj->setParameters($parameters);
    $url = $this->prepareUrl($parameters);
        dpm($url);
    if ($response = $this->request($method_obj->getRequestType(), $url, $parameters)) {
      return $method_obj->processResponse($response);
    }

    return FALSE;
  }

  /**
   * Generate the basic URL
   */
  public function prepareUrl(&$parameters, $upload = FALSE) {
    $parameters['apiKey'] = variable_get('issuu_global_apikey', '');
    $parameters['format'] = 'json';
  
    $this->signRequest($parameters);
  
    $get_array = array();
    foreach ($parameters as $key => $val) {
      $get_array[] = $key . '=' . urlencode($val);
    }
    $get_string = implode('&', $get_array);
  
    $base = ISSUU_URI;
    if ($upload) {
      return array(ISSUU_URI_UPLOAD, $get_string);
    }

    return sprintf('%s?%s', $base, $get_string);
  }

  /**
   * Generate signatures.
   *
   * @see http://issuu.com/services/api/signingrequests.html
   */
  public function signRequest(&$parameters) {
    $signature = '';

    //1.  Sort request parameters alphabetically (e.g. foo=1, bar=2, baz=3 sorts to bar=2, baz=3, foo=1)
    ksort($parameters);

    //2. Concatenate in order your API secret key and request name-value pairs (e.g. SECRETbar2baz3foo1)
    $secret = variable_get('issuu_global_apisecret', '');
    $signature = $secret;
    foreach ($parameters as $k => $v) {
      // file is not part of standard parameters, not documented anywhere, but in this way this works!
      if ($k == 'file') {
        continue;
      }
      $signature .= $k . $v;
    }

    //3. Calculate the signature as the MD5 hash of this string
    $signature = md5($signature);

    //4. Include the signature parameter in the request encoded as lowercase HEX
    $signature = drupal_strtolower($signature);

    $parameters['signature'] = $signature;

    return $signature;
  }

  public function request($type = 'get', $url, $parameters = NULL) {
    if (isset($this->cache[$url])) {
      $response = $this->cache[$url];
    }
    else {
      $type = strtolower($type);
      if ($type == 'post') {
        $response = $this->post($url, $parameters);
      }
      else {
        $response = $this->get($url);
        $response = $response->data;
      }
      $this->cache[$url] = $response;
    }
    
    return $this->processResponse($response);
  }

  public function get($url) {
    $response = drupal_http_request($url);
    return $response;
  }

  public function post($url, $parameters) {
    $ch = curl_init();
  
    curl_setopt($ch, CURLOPT_URL, $url);
    curl_setopt($ch, CURLOPT_POST, TRUE);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, TRUE);
    curl_setopt($ch, CURLOPT_POSTFIELDS, $parameters);
  
    $response = curl_exec($ch);
    curl_close($ch);
    
    return $response;
  }

  public function processResponse($response) {
    try {
      $rsp = drupal_json_decode($response);
    }
    catch (Exception $e) {
      watchdog_exception('issuu', 'Decoding json failed. One possible reason is that issuu.com is down now. The convert failed with exception !exception.', array('!exception' => $e));
      return FALSE;
    }

    if (isset($rsp['rsp'])) {
      $rsp = $rsp['rsp'];
      if ($rsp['stat'] == 'ok') {
        return $rsp['_content']['result'];
      }
      elseif ($rsp['stat'] == 'fail') {
        $this->logError($rsp);
      }
    }
    else {
      watchdog('issuu', 'Unknown error: @response.', array('@response' => $response), WATCHDOG_ERROR);
    }

    return FALSE;
  }

  public function logError($rsp) {
    $error = $rsp['_content']['error'];
    watchdog('issuu', 'Issuu request failed, error code: %code, message: %message', array('%code'=>$error['code'], '%message'=>$error['message']), WATCHDOG_ERROR);
  }
}

abstract class IssuuApiMethod {

  protected $action;

  protected $parameters;

  protected $request_type = 'get';

  public function setParameters($parameters = NULL) {
    $default_parameters = $this->defaultParameters();
    $default_parameters['action'] = $this->action;
    $parameters = array_merge($default_parameters, $parameters);
    $this->parameters = $parameters;
    
    return $parameters;
  }

  public function defaultParameters() {
    return array();
  }

  public function processResponse($response) {
    return $response;
  }

  public function getRequestType() {
    return $this->request_type;
  }
}

class IssuuApiMethod_IssuuDocumentsList extends IssuuApiMethod {
  public function __construct() {
     $this->action = ISSUU_DOCUMENTS_LIST;
  }
}

class IssuuApiMethod_IssuuDocumentsUpload extends IssuuApiMethod {

  public function __construct() {
     $this->action = ISSUU_DOCUMENT_UPLOAD;
     $this->$request_type = 'post';
  }

  function issuu_api_issuu_document_upload($file_location, $parameters=NULL) {
    //TODO make sure file exist
    $default_parameters = array(
      'action' => 'issuu.document.upload',
      'file' => '@' . $file_location,
    );
    $issuu_parameters = array();
    // remove issuu_ prefix used for avoid form item names collision
    if (is_array($parameters)) {
      foreach ($parameters as $name => $param) {
        $issuu_parameters[substr($name, 6)] = $param;
      }
    }
    $parameters = array_merge($default_parameters, (array)$issuu_parameters);
    list($url, $data) = issuu_prepare_url($parameters, TRUE);

    $response = $this->post($url, $parameters);
  
    return $response;
  }

  // public function processResponse($response, &$doc_metadata) {
    // foreach ($rsp['_content']['result']['_content'] as $document) {
      // // get doc values
      // $attributes = array('username', 'name', 'documentId', 'title', 'access', 'state', 'contentFlag', 'category', 'type', 'orgDocType', 'orgDocName', 'origin', 'language', 'rating', 'ratingsAllowed', 'commentCount', 'bookmarkCount', 'viewCount', 'pageCount', 'ep', 'publishDate');
      // foreach ($attributes as $attr) {
        // $doc_metadata[$attr] = (string)$document[$attr];
      // }
    // }
    // return $response;
  // }
}

