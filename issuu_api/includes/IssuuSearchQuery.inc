<?php
/**
 * @file
 * Defines the IssuuSearchQuery and IssuuSearchQueryBuilder class for use in the
 * issuu_api module.
 */

/**
 * Base query class for all Issuu related searches.
 */
class IssuuSearchQuery {

  private $_query_fields;
  private $_subqueries;
  private $_operator;

  //----------------------------------------------------------------------------
  // Constructor
  //----------------------------------------------------------------------------

  /**
   * Create a new instance of IssuuSearchQuery.
   * @param $operator The operator to use when building the query. Valid values
   * are: 'AND' and 'OR'.
   */
  public function __construct($operator) {
    $this->_query_fields = array();
    $this->_subqueries = array();
    // Validate the $operator argument.
    if (isset($operator) && is_string($operator)) {
      // Validate type, continue with operation.
      $this->setOperator($operator);
    }
    else {
      // Invalid type, use AND as default operator.
      $this->setOperator('AND');
    }
  }

  //----------------------------------------------------------------------------
  // Query manipulation methods.
  //----------------------------------------------------------------------------

  /**
   * Add the specified subquery to the current query.
   * @param IssuuSearchQuery $subquery The subquery to add to the current query.
   */
  public function addSubquery(IssuuSearchQuery $subquery) {
    // Add subquery to the subquery list.
    $this->_subqueries[] = $subquery;
  }

  /**
   * Clear all subqueries.
   */
  public function clearSubqueries() {
    $this->_subqueries = array();
  }

  /**
   * Add a query field to the query.
   * @param $name The query field name.
   * @param $value The query field value. Only scalar values ara allowed.
   * @param $require Indicates whether the query field is required.
   */
  public function addQueryField($name, $value, $require = NULL) {
    if (isset($name) && is_string($name) && isset($value) && is_scalar($value)) {
      // Retrieve the query field flag.
      $field_flag = $this->getQueryFieldFlag($require);
      // If the default query contains a space then we need to encapsulate it
      // with quotes.
      if (strpos($value, ' ')) {
        // Encapsulate value with quotes.
        $this->_query_fields[] = "{$field_flag}{$name}:\"{$value}\"";
      }
      else {
        // Add query field.
        $this->_query_fields[] = "{$field_flag}{$name}:{$value}";
      }
    }
  }

  /**
   * Add a ranged query field to the query.
   * @param type $name The query field name.
   * @param $lower_limit The lower limit of the query field. Use '*' to include
   * all values.
   * @param $upper_limit The upper limit of the query field. Use '*' to include
   * all values.
   * @param $require Indicates whether the query field is required.
   */
  public function addRangedQueryField($name, $lower_limit = '*', $upper_limit = '*', $require = NULL) {
    if (isset($name) && is_string($name) && isset($lower_limit) && is_scalar($lower_limit) && isset($upper_limit) && is_scalar($upper_limit)) {
      // Retrieve the query field flag.
      $field_flag = $this->getQueryFieldFlag($require);
      // Add query field.
      $this->_query_fields[] = "{$field_flag}{$name}:[{$lower_limit} TO {$upper_limit}]";
    }
  }

  /**
   * Clear all query fields.
   */
  public function clearQueryFields() {
    $this->_query_fields = array();
  }

  /**
   * Get the query operator.
   * @return Returns the query operator.
   */
  public function getOperator() {
    return $this->_operator;
  }

  /**
   * Set the query operator.
   * @param $operator The operator, allowed values are: 'AND'/'and' and
   * 'OR'/'or'.
   */
  public function setOperator($operator) {
    if (!empty($operator) && is_string($operator)) {
      // Convert operator to uppercase.
      $operator = drupal_strtoupper($operator);
      // Check if the $operator is valid.
      if ($operator == 'AND' || $operator == 'OR') {
        // Set the operator.
        $this->_operator = $operator;
      }
    }
  }

  //----------------------------------------------------------------------------
  // Query helper methods.
  //----------------------------------------------------------------------------

  /**
   * Get the query field flag.
   * @param $require Indicates whether the query field is required.
   * @return Returns the corresponding query field flag.
   */
  protected  function getQueryFieldFlag($require) {
    if (isset($require)) {
      return ($require ? '+' : '-');
    }
    return '';
  }

  //----------------------------------------------------------------------------
  // Query conversion helper methods.
  //----------------------------------------------------------------------------

  /**
   * Get an array of all the build query fields and subqueries.
   * @return Returns an array with all the build query fields and subqueries.
   */
  protected function getQueryParts() {
    $query = $this->_query_fields;
    // Iterate through the subqueries.
    foreach ($this->_subqueries as $subquery) {
      // Build query string for the given subquery and add to the query list.
      $query[] = "({$subquery->__toString()})";
    }
    return $query;
  }

  /**
   * Get the query string.
   * @return Returns the query string.
   */
  public function __toString() {
    // Get the query parts.
    $query = $this->getQueryParts();
    // Build the query string.
    return implode(" {$this->getOperator()} ", $query);
  }

}

/**
 * Extends from the IssuuSearchQuery and provides a default search.
 */
class IssuuSearchQueryBuilder extends IssuuSearchQuery {

  private $_default;

  //----------------------------------------------------------------------------
  // Constructor
  //----------------------------------------------------------------------------

  /**
   * Create a new instance of IssuuSearchQueryBuilder.
   * @param $operator The operator to use when building the query. Valid values
   * are: 'AND' and 'OR'.
   */
  public function __construct($operator) {
    parent::__construct($operator);
    $this->_default = '';
  }

  //----------------------------------------------------------------------------
  // Query builder functions.
  //----------------------------------------------------------------------------

  /**
   * The default query. The meaning of this query depends on the type of search
   * performed.
   * @param $default_query The default query string.
   */
  public function setDefaultQuery($default_query) {
    // Validate function argument.
    if (isset($default_query) && is_string($default_query)) {
      // If the default query contains a space then we need to encapsulate it
      // with quotes.
      if (strpos($default_query, ' ') !== FALSE) {
        // Encapsulate default query with quotes.
        $this->_default = "\"{$default_query}\"";
      }
      else {
        // Set the default query.
        $this->_default = $default_query;
      }
    }
  }

  //----------------------------------------------------------------------------
  // Query conversion helper methods.
  //----------------------------------------------------------------------------

  /**
   * Get the query string.
   * @return Returns the query string.
   */
  public function __toString() {
    $query = empty($this->_default) ? array() : array($this->_default);
    // Merge default query with the query parts.
    $query = array_merge($query, $this->getQueryParts());
    // Build the query string.
    return implode(" {$this->getOperator()} ", $query);
  }

}