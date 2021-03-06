<?php
/**
 * @file
 * Create a Issuu Stream Wrapper class for the Media/Resource module.
 */

/**
 * Stream Wrapper class for Issuu.
 * Create an instance like this:
 * $issuu = new ResourceIssuuStreamWrapper('issuu://document/[issuu-name]');
 */
class MediaIssuuStreamWrapper extends MediaReadOnlyStreamWrapper {
  /**
   * Get MimeType.
   */
  public static function getMimeType($uri, $mapping = NULL) {
    return 'document/issuu';
  }

  /**
   * Get properties.
   *
   * @return array
   *   Array of properties.
   */
  public static function getIssuuProperties($parameters) {
    $properties = array();

    $api_parameters = array(
      'pageSize' => 30,
      'responseParams' => 'name,documentId',
      'resultOrder' => 'desc',
      'startIndex' => 0,
    );
    
    do {
      if ($result = issuu_api_issuu_documents_list($api_parameters)) {
        foreach ($result['_content'] as $document) {
          if ($document['document']['name'] == $parameters['docs']) {
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

    return $properties;
  }

  /**
   * Get target.
   */
  protected function getTarget($uri) {
    return FALSE;
  }

  /**
   * Transform URL.
   */
  public function interpolateUrl() {
    $url = 'issuu.com/' . $this->parameters['document'] . '/docs/' . $this->parameters['docs'];
    return $this->getUrl($url);
  }

  /**
   * Get original thumbnail path.
   */
  public function getOriginalThumbnailPath() {
    $url = 'image.issuu.com/' . $this->parameters['documentId'] . '/jpg/page_1_thumb_large.jpg';
    return $this->getUrl($url);
  }

  /**
   * Get local thumbnail path.
   */
  public function getLocalThumbnailPath() {
    $local_path = 'public://media-issuu/' . $this->parameters['document'] . '-' . $this->parameters['docs'] . '.jpg';
    if (!file_exists($local_path)) {
      $dirname = drupal_dirname($local_path);
      file_prepare_directory($dirname, FILE_CREATE_DIRECTORY | FILE_MODIFY_PERMISSIONS);
      @copy($this->getOriginalThumbnailPath(), $local_path);
    }
    return $local_path;
  }


  public function getUrl($url) {
    global $is_https;
    $protocal = $is_https ? 'https' : 'http';

    return $protocal . '://' . $url;
  }
}

