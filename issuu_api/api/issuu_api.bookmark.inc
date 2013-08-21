<?php
/**
 * @file
 * Defines the Issuu Bookmark API functions.
 */

//------------------------------------------------------------------------------
// Issuu Bookmark API functions.
//------------------------------------------------------------------------------

/**
 * Create a bookmark of any Issuu document. The bookmark can be optionally added
 * to one or more folders.
 * @param array $settings Issuu API settings.
 * @param $document_username Owner of the document.
 * @param $name Name of the document.
 * @param $page Page in document to bookmark. Default is page 1.
 * @param array $folder_ids Folders to add this bookmark to. If no value is
 * submitted the bookmark will not be added to any folder.
 * @return Returns the added bookmark if successful, otherwise an error code is
 * returned.
 */
function issuu_api_bookmark_add(array $settings, $document_username, $name, $page = NULL, array $folder_ids = NULL) {
  try {
    // Construct request parameters.
    $parameters = issuu_api_create_request_parameters();
    $parameters->addStringParam('documentUsername', $document_username);
    $parameters->addStringParam('name', $name);
    $parameters->addIntegerParam('page', $page);
    $parameters->addStringListParam('folderIds', $folder_ids);
    // Create client and perform action.
    return issuu_api_create_client(ISSUU_API_CLIENT_API, $settings)->issuu_bookmark_add($parameters);
  }
  catch (Exception $ex) {
    // Log exception to watchdog.
    watchdog_exception('issuu_api', $ex);
    // Return the error code.
    return $ex->getCode();
  }
}

/**
 * List all bookmarks belonging to a user profile.
 * @param array $settings Issuu API settings.
 * @param $folder_id Folder containing bookmarks to be listed. If this parameter
 * is omitted all bookmarks will be listed regardless of which folder they
 * belong to (if any). Set this parameter to an empty string: '' to list all
 * bookmarks which do not belong to any folder.
 * @param $result_order The result ordering. Default is ascending.
 * @param $start_index Zero based index to start pagination.
 * @param $page_size Maximum number of documents to be returned. Value must be
 * between 0- 30. Default is 10.
 * @param $sort_by The response parameter to sort the result by.
 * @param array $response_parameters An array of response parameters to be
 * returned. If no value is submitted then all parameters will be returned.
 * @return Returns the bookmark list if successful, otherwise an error code is
 * returned.
 */
function issuu_api_bookmarks_list(array $settings, $folder_id = NULL, $result_order = NULL, $start_index = NULL, $page_size = NULL, $sort_by = NULL, array $response_parameters = NULL) {
  try {
    // Construct request parameters.
    $parameters = issuu_api_create_request_parameters();
    $parameters->addStringParam('folderId', $folder_id);
    $parameters->addEnumParam('resultOrder', $result_order, issuu_api_bookmark_result_orders());
    $parameters->addIntegerParam('startIndex', $start_index);
    $parameters->addIntegerParam('pageSize', $page_size);
    $parameters->addEnumParam('bookmarkSortBy', $sort_by, issuu_api_bookmark_response_parameters());
    $parameters->addEnumListParam('responseParams', $response_parameters, issuu_api_bookmark_response_parameters());
    // Create client and perform action.
    return issuu_api_create_client(ISSUU_API_CLIENT_API, $settings)->issuu_bookmarks_list($parameters);
  }
  catch (Exception $ex) {
    // Log exception to watchdog.
    watchdog_exception('issuu_api', $ex);
    // Return the error code.
    return $ex->getCode();
  }
}

/**
 * Update title or description of a bookmark.
 * @param array $settings Issuu API settings.
 * @param $bookmark_id The bookmark to be updated.
 * @param $title New title of the bookmark. If this parameter is omitted then
 * the original title will be kept.
 * @param $description New description of the bookmark. If this field is omitted
 * then the original description will be kept. Max length: 1000 characters.
 * @return Returns the updated bookmark if successful, otherwise an error code
 * is returned.
 */
function issuu_api_bookmark_update(array $settings, $bookmark_id, $title = NULL, $description = NULL) {
  try {
    // Construct request parameters.
    $parameters = issuu_api_create_request_parameters();
    $parameters->addStringParam('bookmarkId', $bookmark_id);
    $parameters->addStringParam('title', $title);
    $parameters->addStringParam('description', $description);
    // Create client and perform action.
    return issuu_api_create_client(ISSUU_API_CLIENT_API, $settings)->issuu_bookmark_update($parameters);
  }
  catch (Exception $ex) {
    // Log exception to watchdog.
    watchdog_exception('issuu_api', $ex);
    // Return the error code.
    return $ex->getCode();
  }
}

/**
 * Delete on or more bookmarks.
 * @param array $settings Issuu API settings.
 * @param array $bookmark_ids An array or bookmark ids.
 * @return Returns TRUE if the bookmarks were deleted, otherwise an error code
 * is returned.
 */
function issuu_api_bookmark_delete(array $settings, array $bookmark_ids) {
  try {
    // Construct request parameters.
    $parameters = issuu_api_create_request_parameters();
    $parameters->addStringListParam('bookmarkIds', $bookmark_ids);
    // Create client and perform action.
    return issuu_api_create_client(ISSUU_API_CLIENT_API, $settings)->issuu_bookmark_delete($parameters);
  }
  catch (Exception $ex) {
    // Log exception to watchdog.
    watchdog_exception('issuu_api', $ex);
    // Return the error code.
    return $ex->getCode();
  }
}

//------------------------------------------------------------------------------
// Issuu Bookmark API parameter definitions.
//------------------------------------------------------------------------------

/**
 * Get the bookmark result orders.
 * @return array Returns an associative array, keyed by the result order.
 */
function issuu_api_bookmark_result_orders() {
  $result_orders = &drupal_static(__FUNCTION__);
  // Check if the array is not already initialized.
  if (!isset($result_orders)) {
    $result_orders = array(
      'asc' => t('Ascending'),
      'desc' => t('Descending'),
    );
  }
  return $result_orders;
}

/**
 * Get the bookmark response parameters.
 * @return array Returns an associative array, keyed by the response parameter.
 */
function issuu_api_bookmark_response_parameters() {
  $response_parameters = &drupal_static(__FUNCTION__);
  // Check if the array is not already initialized.
  if (!isset($response_parameters)) {
    $response_parameters = array(
      'bookmarkId' => t('Unique identifier'),
      'username' => t('Owner'),
      'documentId' => t('Bookmarked document'),
      'documentUsername' => t('Bookmarked document owner'),
      'name' => t('Bookmarked document name'),
      'title' => t('Title'),
      'description' => t('Description'),
      'page' => t('Bookmarked document page'),
      'created' => t('Created'),
      'folders' => t('Folders'),
    );
  }
  return $response_parameters;
}