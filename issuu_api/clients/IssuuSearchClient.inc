<?php
/**
 * @file
 * Defines the IssuuSearchClient class for use in the issuu_api module.
 */

/**
 * Provides the functionality to interact with the Issuu Search API.
 */
class IssuuSearchClient extends IssuuClient {

  const SEARCH_DOCUMENT_ENDPOINT = 'http://search.issuu.com/api/2_0/document';
  const SEARCH_PAGE_ENDPOINT = 'http://search.issuu.com/api/2_0/page';

  //----------------------------------------------------------------------------
  // Constructor
  //----------------------------------------------------------------------------

  /**
   * Create a new instance of IssuuSearchClient.
   */
  public function __construct(array $settings) {
    parent::__construct($settings);
  }

  //----------------------------------------------------------------------------
  // General API interaction functions.
  //----------------------------------------------------------------------------

  /**
   * Perform the search operation agains the given endpoint.
   * @param $endpoint The endpoint to interact with.
   * @param IssuuRequestParams $parameters The request parameters.
   * @return Returns the search results if successful, otherwise an error code
   * is returned.
   */
  protected function search($endpoint, IssuuRequestParams $parameters) {
    if (isset($endpoint) && valid_url($endpoint)) {
      // Perform HTTP GET request to the given $endpoint.
      return $this->get($endpoint, $parameters);
    }
    return ISSUU_API_ERROR_GENERAL_FAULT;
  }

  //----------------------------------------------------------------------------
  // Search API interaction functions.
  //----------------------------------------------------------------------------

  /**
   * Search for documents published on Issuu.
   * @param IssuuRequestParams $parameters The request parameters.
   * @return Returns the search results if successful, otherwise an error code
   * is returned.
   */
  public function issuu_search_document(IssuuRequestParams $parameters) {
    $response = ISSUU_API_ERROR_MISSING_REQUIRED_FIELD;
    // Validate required fields.
    if ($parameters->hasRequestParameter('q')) {
      // Perform the action.
      $response = $this->search(self::SEARCH_DOCUMENT_ENDPOINT, $parameters);
      // Check if the action was successful.
      if (issuu_api_succeeded($response)) {
        $response = $response['response'];
      }
    }
    return $response;
  }

  /**
   * Search within document pages published on Issuu. The parameter documentId
   * or username should be specified.
   * @param IssuuRequestParams $parameters The request parameters.
   * @return Returns the search results if successful, otherwise an error code
   * is returned.
   */
  public function issuu_search_page(IssuuRequestParams $parameters) {
    $response = ISSUU_API_ERROR_MISSING_REQUIRED_FIELD;
    // Validate required fields.
    if ($parameters->hasRequestParameter('q') &&
        ($parameters->hasRequestParameter('documentId') ||
        $parameters->hasRequestParameter('username'))) {
      // Perform the action.
      $response = $this->search(self::SEARCH_PAGE_ENDPOINT, $parameters);
      // Check if the action was successful.
      if (issuu_api_succeeded($response)) {
        $response = $response['response'];
      }
    }
    return $response;
  }

}