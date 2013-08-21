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
  /**
   * Parse URL from Issuu.
   *
   * Issuu has following URL format:
   * http://issuu.com/user/docs/
   */
  public function parse($embed_code) {
    $patterns = array(
      '@issuu\.com/webembed/.*\?.*documentId=([0-9]{12}-\w{32})@i',
    );
    // Parse patterns[0]: embed code.
    watchdog('media_issuu patterns[0]', $patterns[0]);
    preg_match($patterns[0], $embed_code, $matches);
    if (isset($matches[1])) {
      $request = 'http://search.issuu.com/api/2_0/document?q=documentId:' . $matches[1];
      $response = drupal_http_request($request);
      dsm($response);
      watchdog('media_issuu response', '<pre>' . print_r($response, TRUE) . '</pre>');
      $decodedresponse = drupal_json_decode($response->data);
      watchdog('media_issuu decodedresponse', '<pre>' . print_r($decodedresponse, TRUE) . '</pre>');
      dsm($decodedresponse);
      if ($decodedresponse['response']['numFound'] === 1) {
        return file_stream_wrapper_uri_normalize('issuu://document' . '/' . $decodedresponse['response']['docs'][0]['username'] . '/docs/' .  $decodedresponse['response']['docs'][0]['docname']);
      }
      else {
        return FALSE;
      }
    }
    // Parse patterns[1]: url.
    $parts = parse_url($embed_code);
    $pathparts = explode('/', $parts['path']);
    if (count($pathparts) === 4 && isset($pathparts[1]) && $pathparts[2] === 'docs' && isset($parts['host']) && $parts['host'] === 'issuu.com') {
      return file_stream_wrapper_uri_normalize('issuu://document' . $parts['path']);
    }
    return FALSE;
  }
  /**
   * Get file object.
   */
  public function getFileObject() {
    $uri = $this->parse($this->embedCode);
    $file = file_uri_to_object($uri, TRUE);
    dsm($file);
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
