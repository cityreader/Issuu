<?php
/**
 * @file
 * Defines the IssuuApiFormatter which is responsible for converting the request
 * and response data to and from the Issuu service.
 */

/**
 * Request and response formatter for the Issuu service.
 *
 * TODO: Split this class into IssuuApiRequestFormatter and
 * IssuuApiResponseFormatter after the http_client issue get submitted.
 * (http://drupal.org/node/1351014)
 */
class IssuuApiFormatter implements HttpClientFormatter {

  /**
   * Serializes an associative array.
   * @param $data The data that should be serialized.
   * @return The serialized data as a string.
   * @throws InvalidArgumentException
   */
  public function serialize($data) {
    // Validate function argument.
    if (is_array($data) && isset($data['parameters']) && is_a($data['parameters'], 'IssuuRequestParams')) {
      // Retrieve IssuuRequestParams object.
      $parameters = $data['parameters'];
      // Return the request parameter array if multipart else create query
      // string.
      return (empty($data['multipart']) ? http_build_query($parameters->getRequestParameters()) : $parameters->getRequestParameters());
    }
    throw new InvalidArgumentException('Formatter can only serialize IssuuRequestParams objects.', ISSUU_API_ERROR_GENERAL_FAULT);
  }

  /**
   * Unserializes JSON data.
   * @param $data The data that should be unserialized.
   * @return The unserialized data.
   * @throws InvalidArgumentException
   */
  public function unserialize($data) {
    // Check if it can be converted.
    if (($decoded = drupal_json_decode($data)) !== NULL) {
      return $decoded;
    }
    throw new InvalidArgumentException('JSON response was malformed.', ISSUU_API_ERROR_GENERAL_FAULT);
  }

  /**
   * Get the MIME type.
   * @return Returns the MIME type.
   */
  public function mimeType() {
    return 'application/json';
  }

}
