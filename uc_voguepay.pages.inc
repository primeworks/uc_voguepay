<?php

/**
 * @file
 * Callback functions to handle responses from VoguePay.
 */

/**
 * Handles VoguePay payment notification callback.
 */
function uc_voguepay_callback() {
  if (isset($_POST['serial-number'])) {
    $request = '<?xml version="1.0" encoding="UTF-8"?>';
    $request .= "\n";
    // TODO: This looks interesting. Can it be tied into VoguePay?
    $request .= '<notification-history-request xmlns="http://checkout.google.com/schema/2">';
    $request .= '<serial-number>'. check_plain($_POST['serial-number']) .'</serial-number>';
    $request .= '</notification-history-request>';

    if ($response = uc_voguepay_send_request('reports', $request)) {
      watchdog('voguepay', 'Notification document: @xml', array('@xml' => $response->asXML()), WATCHDOG_DEBUG);
      switch ($response->getName()) {
        case 'new-order-notification':
          $success = uc_voguepay_new_order($response);
        break;
        case 'order-state-change-notification':
          $success = uc_voguepay_order_state_change($response);
        break;
        case 'risk-information-notification':
          $success = uc_voguepay_accept_risk($response);
        break;
        case 'authorization-amount-notification':
          $success = uc_voguepay_authorize_amount($response);
        break;
        case 'charge-amount-notification':
          $success = uc_voguepay_charge_order($response);
        break;
        case 'refund-amount-notification':
          $success = uc_voguepay_refund_order($response);
        break;
        default:
          $success = TRUE;
        break;
      }

      if ($success) {
        uc_voguepay_notification_acknowledgement($_POST['serial-number']);
      }
    }

    uc_voguepay_notification_error();
  }

  return MENU_NOT_FOUND;
}

/**
 * Verifies VoguePay shipping calculation.
 */

 // TODO: Can we build this into VoguePay?
function uc_voguepay_merchant_calculation() {
  // Error checking.
  if (variable_get('uc_voguepay_mode', 'checkout') == 'checkout') {
    $merchant_id = variable_get('uc_voguepay_merchant_id', '');
    $merchant_key = variable_get('uc_voguepay_merchant_key', '');
  }
  else {
    $merchant_id = variable_get('uc_voguepay_test_merchant_id', '');
    $merchant_key = variable_get('uc_voguepay_test_merchant_key', '');
  }

  //watchdog('voguepay', check_plain($_SERVER['PHP_AUTH_USER']), NULL, WATCHDOG_NOTICE);
  if ($_SERVER['PHP_AUTH_USER'] != $merchant_id || $_SERVER['PHP_AUTH_PW'] != $merchant_key) {
    watchdog('voguepay', 'HTTP Authorization header does not match settings.', array(), WATCHDOG_ERROR);
    return MENU_ACCESS_DENIED;
  }

  $input = file_get_contents('php://input');
  $calc = new SimpleXMLElement($input);

  if ($calc->getName() != 'merchant-calculation-callback') {
    uc_voguepay_notification_error();
  }

  $order = new stdClass();
  $order->uid = 0;

  $products = array();
  foreach ($calc->{'shopping-cart'}->items->item as $item) {
    list($nid, $model) = explode('|', $item->{'merchant-item-id'});
    $node = node_load($nid);
    $node->model = (string) $item->{'item-name'};
    $node->title = (string) $item->{'item-description'};
    $node->price = (string) $item->{'unit-price'};
    $node->qty = (string) $item->quantity;
    $node->weight = (string) $item->{'item-weight'};

    $products[] = $node;
  }

  $order->products = $products;

  $address = $calc->calculate->addresses->{'anonymous-address'};

  $country = uc_get_country_data(array('country_iso_code_2' => $address->{'country-code'}));
  $order->delivery_country = $country[0]['country_id'];
  $order->delivery_city = $address->city;
  $zone_id = db_result(db_query("SELECT zone_id FROM {uc_zones} WHERE zone_code = '%s' AND zone_country_id = %d", $address->region, $order->delivery_country));
  $order->delivery_zone = $zone_id;
  $order->delivery_postal_code = $address->{'postal-code'};

  $order->order_total = uc_order_get_total($order, TRUE);

  $methods = module_invoke_all('shipping_method');
  module_load_include('inc', 'uc_quote', 'uc_quote.pages');
  $quote_data = _uc_quote_assemble_quotes($order);
  $results = '';
  foreach ($quote_data as $method_id => $options) {
    foreach ($options as $accsrl => $data) {
      $results .= '<result shipping-name="'. $methods[$method_id]['quote']['accessorials'][$accsrl] .'" address-id="'. $address['id'] .'">';

      $results .= '<shipping-rate currency="'. variable_get('uc_currency_code', 'USD') .'">'. $data['rate'] .'</shipping-rate>';
      $results .= '<shippable>true</shippable>';

      $results .= '</result>';
    }
  }

  if ($results) {
    $response = '<?xml version="1.0" encoding="UTF-8"?>' . "\n";
    $response .= '<merchant-calculation-results xmlns="http://checkout.google.com/schema/2">';
    $response .= '<results>';
    $response .= $results;
    $response .= '</results>';
    $response .= '</merchant-calculation-results>';

    drupal_set_header('HTTP/1.1 200 OK');
    print $response;
    exit();
  }

  drupal_set_header('HTTP/1.1 204 No Content');
  exit();
  // So there!
}

/**
*Helper code for VoguePay
*
*/

function _uc_voguepay_send_to_vp( $type, $param ) {
  $vp_url = "https://www.voguepay.com";
  if( $type == 'init' ) $vp_url .= '/pay';
  else if( $type == 'complete' ) $vp_url .= '/pay';
  else return array('curl_status' => 'invalid');

  $data = '';
  foreach ($param as $key => $value) {
    $data .= $key .'='. urlencode(str_replace(',', '', $value)) .'&';
  }

  $data = substr($data, 0, -1);
  $ch = curl_init();
  curl_setopt($ch, CURLOPT_URL, $vp_url);
  curl_setopt($ch, CURLOPT_VERBOSE, 0);
  curl_setopt($ch, CURLOPT_POST, 1);
  curl_setopt($ch, CURLOPT_POSTFIELDS, $data);
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
  curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, variable_get('uc_voguepay_curl_ssl_verifypeer',1));
  curl_setopt($ch, CURLOPT_CAINFO, variable_get('uc_voguepay_curl_ca_cert_bundle',''));
  curl_setopt($ch, CURLOPT_NOPROGRESS, 1);
  curl_setopt($ch, CURLOPT_FOLLOWLOCATION, 0);
  $response = json_decode(curl_exec($ch));
  if ($error = curl_error($ch)) {
    $return = array('curl_status' => 'failed', 'error' => $error );
  }
  else $return = array('curl_status' => 'success', 'response' => $response );
  curl_close($ch);
  return $return;
} // end - _uc_voguepay_send_to_vp
