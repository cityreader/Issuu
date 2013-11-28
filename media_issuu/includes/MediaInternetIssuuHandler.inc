<?php
/**
 * @file
 * media_internet handler for issuu.
 */

/**
 * Implementation of MediaInternetBaseHandler.
 *
 * @see hook_media_internet_providers()
 */
class MediaInternetIssuuHandler extends MediaInternetBaseHandler {
  
  public $properties;

  /**
   * Parse URL from Issuu.
   *
   * Issuu has following URL format:
   * http://issuu.com/username/docs/docName?e=configid
   */
  public function parse($embed_code) {
    $cache = &drupal_static(__METHOD__);

    if (!isset($cache[$embed_code])) {
      $result = FALSE;
      // $patterns = array(
        // '@issuu\.com/.*\?.*documentId=([0-9]{12}-\w{32})@i',
      // );
      // // Parse patterns[0]: embed code.
      // watchdog('media_issuu patterns[0]', $patterns[0]);
      // preg_match($patterns[0], $embed_code, $matches);
      // if (isset($matches[1])) {
        // $request = 'http://search.issuu.com/api/2_0/document?q=documentId:' . $matches[1];
        // $response = drupal_http_request($request);
        // dsm($response);
        // watchdog('media_issuu response', '<pre>' . print_r($response, TRUE) . '</pre>');
        // $decodedresponse = drupal_json_decode($response->data);
        // watchdog('media_issuu decodedresponse', '<pre>' . print_r($decodedresponse, TRUE) . '</pre>');
        // dsm($decodedresponse);
        // if ($decodedresponse['response']['numFound'] === 1) {
          // return file_stream_wrapper_uri_normalize('issuu://document' . '/' . $decodedresponse['response']['docs'][0]['username'] . '/docs/' .  $decodedresponse['response']['docs'][0]['docname']);
        // }
        // else {
          // return FALSE;
        // }
      // }
      // Parse patterns[1]: url.
      $pattern = '@issuu\.com/([^/"\&\? ]+)/docs/([^/"\&\? ]+)\?e=(\d*/\d{7})@i';
  
      watchdog('media_issuu patterns[0]', $pattern);
      preg_match($pattern, $embed_code, $matches);
  
      if (isset($matches[1])) {
  
        if ($properties = $this->getIssuuProperties($matches[2])) {
          $uri = sprintf('issuu://document/%s/docs/%s/documentId/%s/configid/%s',
                 $matches[1],
                 $matches[2],
                 $properties['documentId'],
                 rawurlencode($matches[3]));
          $result = file_stream_wrapper_uri_normalize($uri);
        }
      }      
      $cache[$embed_code]['value'] = $result;
      $cache[$embed_code]['properties'] = $properties;
    }

    if (empty($this->properties)) {
      $this->properties = $cache[$embed_code]['properties'];
    }

    return $cache[$embed_code]['value'];
  }

  /**
   * Get properties.
   *
   * @return array
   *   Array of properties.
   */
  public function getIssuuProperties($docName) {
      $properties = array();
  
      $api_parameters = array(
        'pageSize' => 30,
        'responseParams' => 'name,documentId,title',
        'resultOrder' => 'desc',
        'startIndex' => 0,
      );
      
      do {
        if ($result = issuu_api_issuu_documents_list($api_parameters)) {
          foreach ($result['_content'] as $document) {
            if ($document['document']['name'] == $docName) {
              $properties = $document['document'];
              break;
            }
          }
          $api_parameters['startIndex'] += 30;
        }
        else {
          break;
        }
      }
      while (empty($properties) || $result['more'] == TRUE);

      $this->properties = $properties;

    return $this->properties;
  }

  /**
   * Get file object.
   */
  public function getFileObject() {
    $uri = $this->parse($this->embedCode);
    $file = file_uri_to_object($uri, TRUE);

    if (isset($this->properties)) {
      $file->filename = truncate_utf8($this->properties['title'], 255);
    }

    return $file;
  }

  /**
   * Claim parsed embedCode.
   */
  public function claim($embed_code) {
    if ($this->parse($embed_code)) {
      return TRUE;
    }
  }

  /**
   * Save file.
   */
  public function save() {
    $file = $this->getFileObject();
    file_save($file);
    return $file;
  }
}

