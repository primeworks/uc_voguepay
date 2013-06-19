<?php
function voguepay_help($path, $arg) {
    //$output = '<p>'.  t("voguepay is a module that extends functionality of sms framework.");
    //    The line above outputs in ALL admin/module pages
    switch ($path) {
        case "admin/help/voguepay":
        $output = '<p>'.  t("eCommerce - Voguepay - Drupal Plugin.") .'</p>';
            break;
    }
    return $output;
} // function voguepay_help

/**
 * Valid permissions for this module
 * @return array An array of valid permissions for the voguepay module
 */
function voguepay_perm() {
    return array('administer voguepay');
} // function voguepay_perm()

/**
 * Menu for this module
 * @return array An array with this module's settings.
 */
function voguepay_menu() {
    $items = array();


      //Link to the sms_zone admin page:
    $items['voguepay'] = array(
        'title' => 'Voguepay',
        'description' => 'Voguepay - Drupal Plugin',

		'page callback'    => 'drupal_get_form',
        'page arguments'   => array('voguepay_form'),

        'access arguments' => array('administer nodes'),
        'type' => MENU_NORMAL_ITEM,
    );
	

    return $items;
}




function voguepay_form() {
   $form['merchant'] = array(
      '#type' => 'textfield', 
      '#title' => t('VoguePay Merchant ID'), 
      '#default_value' => variable_get('vogue_merchant',''), 
      '#description' => t(''),
      '#required' => TRUE
	  );
	  
	$form['color'] = array(
   '#type' => 'select', 
   '#title' => t('Button Color'), 
   '#default_value' => variable_get('vogue_color','blue'),
   '#options' => array(
        'blue' => t('Blue'), 
        'red' => t('Red'), 
        'green' => t('Green'), 
        'grey' => t('Grey'), 
      ),
   '#description' => t(''),
      '#required' => FALSE
    );

   $form['custom'] = array(
      '#type' => 'textfield', 
      '#title' => t('Custom button'), 
      '#default_value' => variable_get('vogue_custom',''), 
      '#description' => t(''),
      '#required' => FALSE
	  );


    $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save Changes'),
  );


	  return $form;
}


function voguepay_form_submit(&$form, $form_state) {

$merchant=$form_state['values']['merchant'];
$color=$form_state['values']['color'];
$custom=$form_state['values']['custom'];

variable_set('vogue_merchant',$merchant);
variable_set('vogue_color',$color);
variable_set('vogue_custom',$custom);


drupal_set_message(t("Your changes were saved successfully."));
  
$form_state['redirect'] = 'voguepay';
}

function voguepay_nodeapi(&$node, $op, $a3 = NULL, $a4 = NULL)  {
		
		
		
if($op=="alter")	 {		

 $sitename  = variable_get('site_name', '');
 
         // Get the body
        $body = $node->body;

        // Regular expression to fetch the voguepay tags
		$regex = '/{voguepay\s*.*?}/i';
		preg_match_all( $regex, $body, $matches );

		
        // Fetch the default parameters
        $merchant_id= variable_get('vogue_merchant','');
        $alternate_button = variable_get('vogue_custom','');
        $button_color = variable_get('vogue_color','blue');
        $button = empty($alternate_button) ? 'http://voguepay.com/images/buttons/buynow_'.$button_color.'.png' : $alternate_button ;
			
			
		foreach($matches[0] as $key => $match) {

			$pattern = '/item\s*\((?<val>[^\(\)]+)\)/';
			preg_match($pattern, $match, $m);
			$item = $m['val'];
			$pattern = '/price\s*\((?<val>[^\(\)]+)\)/';
			preg_match($pattern, $match, $m);
			$price = $m['val'];
			$pattern = '/description\s*\((?<val>[^\(\)]+)\)/';
			preg_match($pattern, $match, $m);
			$description = empty($m['val']) ? $item.' at '.number_format($price,2) : $m['val'];
			
			$f = '<form method="POST" action="https://voguepay.com/pay/">
			<input type="hidden" name="v_merchant_id" value="'.$merchant_id.'" />
			<input type="hidden" name="memo" value="'.$item.' ('.number_format($price,2).') order from '.$sitename.'" />
			<input type="hidden" name="item_1" value="'.$item.'" />
			<input type="hidden" name="description_1" value="'.$description.'" />
			<input type="hidden" name="price_1" value="'.$price.'" />
			<input type="hidden" name="total" value="'.$price.'" />
			<input type="image" src="'.$button.'" alt="Pay with VoguePay" />
			</form>';
			
            $body = str_replace($match,$f,$body);
		}
		
		$node->body=$body;
}
		
}



