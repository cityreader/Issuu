<?php
/**
 * @file
 * Defines the Issuu Search API functions.
 */

//------------------------------------------------------------------------------
// Issuu Search API functions.
//------------------------------------------------------------------------------

/**
 * Search for documents published on Issuu.
 * @param array $settings Issuu search settings.
 * @param IssuuSearchQueryBuilder $query The Issuu search query.
 * @param $timeframe Search in document from within the specified time interval.
 * @param $explicit Search explicit documents. If omitted then default FALSE is
 * used.
 * @param $language Restrict the search to documents categorized with the
 * submitted language code.
 * @param $start_index Zero based index to start pagination.
 * @param $page_size Maximum number of results to be returned. Value must be
 * between 0- 30. Default is 10.
 * @param $sort_by Field to sort the search results by.
 * @param $sort_direction Sort direction of the search results. If the sort_by
 * parameter is omitted then the this parameter is ignored.
 * @param array $response_parameters An array of response parameters to be
 * returned. If no value is submitted then the minimum of required parameters
 * are returned.
 * @return Returns the search results if successful, otherwise an error code
 * is returned.
 */
function issuu_api_search_document(array $settings, IssuuSearchQueryBuilder $query, $document_id = NULL, $username = NULL, $timeframe = NULL, $explicit = NULL, $language = NULL, $start_index = NULL, $page_size = NULL, $sort_by = NULL, $sort_direction = NULL, array $response_parameters = NULL) {
  try {
    // Construct request parameters.
    $parameters = issuu_api_create_request_parameters();
    $parameters->addStringParam('q', $query->__toString());
    $parameters->addStringParam('documentId', $document_id);
    $parameters->addStringParam('username', $username);
    $parameters->addEnumParam('timeframe', $timeframe, issuu_api_search_timeframes());
    $parameters->addBooleanIntegerParam('explicit', $explicit);
    if (isset($language)) {
      // The $language is specified, include the Issuu Document API so we can
      // access the issuu_api_document_language_codes() function which contains
      // all valid language codes for a document.
      issuu_api_include(ISSUU_API_DOCUMENT);
      $parameters->addEnumParam('language', $language, issuu_api_document_language_codes());
    }
    $parameters->addIntegerParam('startIndex', $start_index);
    $parameters->addIntegerParam('pageSize', $page_size);
    if (isset($sort_by)) {
      if (isset($sort_direction)) {
        // $sort_direction is set, due to the special format (e.g.
        // "$sort_by $sort_direction") we cannot use the parameter validation
        // provided by the request parameters object.
        $allowed_sort_direction_values = issuu_api_search_sort_directions();
        // Validate $sort_direction.
        if (empty($sort_direction) || !is_string($sort_direction) || !isset($allowed_sort_direction_values[$sort_direction])) {
          throw new InvalidArgumentException('Invalid sort direction was specified.', ISSUU_API_ERROR_INVALID_FIELD_FORMAT);
        }
        $allowed_sort_by_values = issuu_api_search_document_sort_fields();
        // Validate $sort_by.
        if (empty($sort_by) || !is_string($sort_by) || !isset($allowed_sort_by_values[$sort_by])) {
          throw new InvalidArgumentException('Parameter "sortBy" must contain a value defined by the enumeration.', ISSUU_API_ERROR_INVALID_FIELD_FORMAT);
        }
        // Validation for $sort_direction and $sort_by was successful, add the
        // parameter.
        $parameters->addParam('sortBy', "{$sort_by} {$sort_direction}");
      }
      else {
        // Sort direction is not set, validate the parameter using the request
        // parameters object.
        $parameters->addEnumParam('sortBy', $sort_by, issuu_api_search_document_sort_fields());
      }
    }
    $parameters->addEnumListParam('responseParams', $response_parameters, issuu_api_search_document_response_parameters());
    // Create client and perform action.
    return issuu_api_create_client(ISSUU_API_CLIENT_SEARCH, $settings)->issuu_search_document($parameters);
  }
  catch (Exception $ex) {
    // Log exception to watchdog.
    watchdog_exception('issuu_api', $ex);
    // Return the error code.
    return $ex->getCode();
  }
}

/**
 * Search within document pages published on Issuu. Either the document id or
 * username should be specified.
 * @param array $settings Issuu search settings.
 * @param IssuuSearchQueryBuilder $query The Issuu search query.
 * @param $document_id The document which will be searched.
 * @param $username Search within a user's public documents if submitted.
 * @param $timeframe Search in document from within the specified time interval.
 * @param $explicit Search explicit documents. If omitted then default FALSE is
 * used.
 * @param $language Restrict the search to documents categorized with the
 * submitted language code.
 * @param $start_index Zero based index to start pagination.
 * @param $page_size Maximum number of results to be returned. Value must be
 * between 0- 30. Default is 10.
 * @param $sort_by Field to sort the search results by.
 * @param $sort_direction Sort direction of the search results. If the sort_by
 * parameter is omitted then the this parameter is ignored.
 * @param array $response_parameters An array of response parameters to be
 * returned. If no value is submitted then the minimum of required parameters
 * are returned.
 * @return Returns the search results if successful, otherwise an error code
 * is returned.
 */
function issuu_api_search_page(array $settings, IssuuSearchQueryBuilder $query, $document_id = NULL, $username = NULL, $timeframe = NULL, $explicit = NULL, $language = NULL, $start_index = NULL, $page_size = NULL, $sort_by = NULL, $sort_direction = NULL, array $response_parameters = NULL) {
  try {
    // Construct request parameters.
    $parameters = issuu_api_create_request_parameters();
    $parameters->addStringParam('q', $query->__toString());
    $parameters->addStringParam('documentId', $document_id);
    $parameters->addStringParam('username', $username);
    $parameters->addEnumParam('timeframe', $timeframe, issuu_api_search_timeframes());
    $parameters->addBooleanIntegerParam('explicit', $explicit);
    if (isset($language)) {
      // The $language is specified, include the Issuu Document API so we can
      // access the issuu_api_document_language_codes() function which contains
      // all valid language codes for a document.
      issuu_api_include(ISSUU_API_DOCUMENT);
      $parameters->addEnumParam('language', $language, issuu_api_document_language_codes());
    }
    $parameters->addIntegerParam('startIndex', $start_index);
    $parameters->addIntegerParam('pageSize', $page_size);
    if (isset($sort_by)) {
      if (isset($sort_direction)) {
        // $sort_direction is set, due to the special format (e.g.
        // "$sort_by $sort_direction") we cannot use the parameter validation
        // provided by the request parameters object.
        $allowed_sort_direction_values = issuu_api_search_sort_directions();
        // Validate $sort_direction.
        if (empty($sort_direction) || !is_string($sort_direction) || !isset($allowed_sort_direction_values[$sort_direction])) {
          throw new InvalidArgumentException('Invalid sort direction was specified.', ISSUU_API_ERROR_INVALID_FIELD_FORMAT);
        }
        $allowed_sort_by_values = issuu_api_search_page_sort_fields();
        // Validate $sort_by.
        if (empty($sort_by) || !is_string($sort_by) || !isset($allowed_sort_by_values[$sort_by])) {
          throw new InvalidArgumentException('Parameter "sortBy" must contain a value defined by the enumeration.', ISSUU_API_ERROR_INVALID_FIELD_FORMAT);
        }
        // Validation for $sort_direction and $sort_by was successful, add the
        // parameter.
        $parameters->addParam('sortBy', "{$sort_by} {$sort_direction}");
      }
      else {
        // Sort direction is not set, validate the parameter using the request
        // parameters object.
        $parameters->addEnumParam('sortBy', $sort_by, issuu_api_search_page_sort_fields());
      }
    }
    $parameters->addEnumListParam('responseParams', $response_parameters, issuu_api_search_page_response_parameters());
    // Create client and perform action.
    return issuu_api_create_client(ISSUU_API_CLIENT_SEARCH, $settings)->issuu_search_page($parameters);
  }
  catch (Exception $ex) {
    // Log exception to watchdog.
    watchdog_exception('issuu_api', $ex);
    // Return the error code.
    return $ex->getCode();
  }
}

//------------------------------------------------------------------------------
// Issuu Search API parameter definitions.
//------------------------------------------------------------------------------

/**
 * Get the search timeframes.
 * @return array Returns an associative array, keyed by the timeframe.
 */
function issuu_api_search_timeframes() {
  $timeframes = &drupal_static(__FUNCTION__);
  // Check if the array is not already initialized.
  if (!isset($timeframes)) {
    $timeframes = array(
      '1day' => t('1 day'),
      '1week' => t('1 week'),
      '1month' => t('1 month'),
      '1year' => t('1 year'),
    );
  }
  return $timeframes;
}

/**
 * Get the search sort directions.
 * @return array Returns an associative array, keyed by the sort direction.
 */
function issuu_api_search_sort_directions() {
  $sort_directions = &drupal_static(__FUNCTION__);
  // Check if the array is not already initialized.
  if (!isset($sort_directions)) {
    $sort_directions = array(
      'asc' => t('Ascending'),
      'desc' => t('Descending'),
    );
  }
  return $sort_directions;
}

/**
 * Get the sortable document fields.
 * @return array Returns an associative array, keyed by the sortable field.
 */
function issuu_api_search_document_sort_fields() {
  $sort_fields = &drupal_static(__FUNCTION__);
  // Check if the array is not already initialized.
  if (!isset($sort_fields)) {
    $sort_fields = array(
      'documentId' => t('Unique identifier'),
      'pagecount' => t('Page count'),
      'docname' => t('Name'),
      'username' => t('Owner'),
      'views' => t('View count'),
      'tag' => t('Tags'),
      'rating' => t('Rating'),
      'language' => t('Language'),
    );
  }
  return $sort_fields;
}

/**
 * Get the sortable page fields.
 * @return array Returns an associative array, keyed by the sortable field.
 */
function issuu_api_search_page_sort_fields() {
  $sort_fields = &drupal_static(__FUNCTION__);
  // Check if the array is not already initialized.
  if (!isset($sort_fields)) {
    $sort_fields = array(
      'documentId' => t('Unique identifier'),
      'pageId' => t('Page number'),
      'pagecount' => t('Page count'),
      'docname' => t('Name'),
      'username' => t('Owner'),
      'views' => t('View count'),
      'tag' => t('Tags'),
      'rating' => t('Rating'),
      'language' => t('Language'),
    );
  }
  return $sort_fields;
}

/**
 * Get the search document response parameters.
 * @return array Returns an associative array, keyed by the response parameter.
 */
function issuu_api_search_document_response_parameters() {
  $response_parameters = &drupal_static(__FUNCTION__);
  // Check if the array is not already initialized.
  if (!isset($response_parameters)) {
    $response_parameters = array(
      'title' => t('Title'),
      'documentId' => t('Unique identifier'),
      'pagecount' => t('Page count'),
      'docname' => t('Name'),
      'username' => t('Owner'),
      'description' => t('Description'),
      'views' => t('View count'),
      'epoch' => t('Created'),
      'tag' => t('Tags'),
      'rating' => t('Rating'),
    );
  }
  return $response_parameters;
}

/**
 * Get the search page response parameters.
 * @return array Returns an associative array, keyed by the response parameter.
 */
function issuu_api_search_page_response_parameters() {
  $response_parameters = &drupal_static(__FUNCTION__);
  // Check if the array is not already initialized.
  if (!isset($response_parameters)) {
    $response_parameters = array(
      'title' => t('Title'),
      'documentId' => t('Unique identifier'),
      'pageId' => t('Page number'),
      'pagecount' => t('Page count'),
      'docname' => t('Name'),
      'username' => t('Owner'),
      'description' => t('Description'),
      'views' => t('View count'),
      'epoch' => t('Created'),
      'tag' => t('Tags'),
      'rating' => t('Rating'),
    );
  }
  return $response_parameters;
}