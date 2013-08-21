<?php
/**
 * @file
 * Defines the Issuu Folder API functions.
 */

//------------------------------------------------------------------------------
// Issuu Folder API functions.
//------------------------------------------------------------------------------

/**
 * Create an empty folder. The folder can later be filled with bookmarks of any
 * document on Issuu.
 * @param array $settings Issuu API settings.
 * @param $name Name of the folder. Must be different from other folder names.
 * @param $description Description of folder content.
 * @return Returns the added folder if successful, otherwise an error code is
 * returned.
 */
function issuu_api_folder_add(array $settings, $name, $description) {
  try {
    // Construct request parameters.
    $parameters = issuu_api_create_request_parameters();
    $parameters->addStringParam('folderName', $name);
    $parameters->addStringParam('folderDescription', $description);
    // Create client and perform the document update action.
    return issuu_api_create_client(ISSUU_API_CLIENT_API, $settings)->issuu_folder_add($parameters);
  }
  catch (Exception $ex) {
    // Log exception to watchdog.
    watchdog_exception('issuu_api', $ex);
    // Return the error code.
    return $ex->getCode();
  }
}

/**
 * List all folders belonging to a user profile.
 * @param array $settings Issuu API settings.
 * @param $result_order The result ordering. Default is ascending.
 * @param $start_index Zero based index from where to start pagination.
 * @param $page_size Maximum number of folders to be returned. Value must be
 * between 0 - 30. Default is 10.
 * @param $sort_by Response parameter to sort the result by.
 * @param array $response_parameters An array of response parameters to be
 * returned. If no value is submitted then all parameters will be returned.
 * @return Returns the folder list if successful, otherwise an error code is
 * returned.
 */
function issuu_api_folders_list(array $settings, $result_order = NULL, $start_index = NULL, $page_size = NULL, $sort_by = NULL, array $response_parameters = NULL) {
  try {
    // Construct request parameters.
    $parameters = issuu_api_create_request_parameters();
    $parameters->addEnumParam('resultOrder', $result_order, issuu_api_folder_result_orders());
    $parameters->addIntegerParam('startIndex', $start_index);
    $parameters->addIntegerParam('pageSize', $page_size);
    $parameters->addEnumParam('folderSortBy', $sort_by, issuu_api_folder_response_parameters());
    $parameters->addEnumListParam('responseParams', $response_parameters, issuu_api_folder_response_parameters());
    // Create client and perform action.
    return issuu_api_create_client(ISSUU_API_CLIENT_API, $settings)->issuu_folders_list($parameters);
  }
  catch (Exception $ex) {
    // Log exception to watchdog.
    watchdog_exception('issuu_api', $ex);
    // Return the error code.
    return $ex->getCode();
  }
}

/**
 * Update folder name or description.
 * @param array $settings Issuu API settings.
 * @param $folder_id The folder to be updated.
 * @param $name New name of the folder. If this parameter is omitted then the
 * original name will be kept.
 * @param $description New description of the folder. If this parameter is
 * omitted then the original name will be kept.
 * @return Returns the updated folder if successful, otherwise an error code is
 * returned.
 */
function issuu_api_folder_update(array $settings, $folder_id, $name = NULL, $description = NULL) {
  try {
    // Construct request parameters.
    $parameters = issuu_api_create_request_parameters();
    $parameters->addStringParam('folderId', $folder_id);
    $parameters->addStringParam('folderName', $name);
    $parameters->addStringParam('folderDescription', $description);
    // Create client and perform action.
    return issuu_api_create_client(ISSUU_API_CLIENT_API, $settings)->issuu_folder_update($parameters);
  }
  catch (Exception $ex) {
    // Log exception to watchdog.
    watchdog_exception('issuu_api', $ex);
    // Return the error code.
    return $ex->getCode();
  }
}

/**
 * Delete one or more folders.
 * @param array $settings Issuu API settings.
 * @param array $folder_ids An array of folder ids.
 * @return Returns TRUE if the folders were deleted, otherwise an error code is
 * returned.
 */
function issuu_api_folder_delete(array $settings, array $folder_ids = NULL) {
  try {
    // Construct request parameters.
    $parameters = issuu_api_create_request_parameters();
    $parameters->addStringListParam('folderIds', $folder_ids);
    // Create client and perform action.
    return issuu_api_create_client(ISSUU_API_CLIENT_API, $settings)->issuu_folder_delete($parameters);
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
 * Get the folder result orders.
 * @return array Returns an associative array, keyed by the result order.
 */
function issuu_api_folder_result_orders() {
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
 * Get the folder response parameters.
 * @return array Returns an associative array, keyed by the response parameter.
 */
function issuu_api_folder_response_parameters() {
  $response_parameters = &drupal_static(__FUNCTION__);
  // Check if the array is not already initialized.
  if (!isset($response_parameters)) {
    $response_parameters = array(
      'folderId' => t('Unique identifier'),
      'username' => t('Owner'),
      'name' => t('Name'),
      'description' => t('Description'),
      'items' => t('Number of bookmarks'),
      'created' => t('Created'),
    );
  }
  return $response_parameters;
}