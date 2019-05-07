<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Menu 
{
	private $arr_menu;
	public function __construct($other_array)
	{
		$this->arr_menu = $other_array;
	}

	public function BuildMenu()
	{	
		$text = '<nav><ul>';
		foreach($this->arr_menu as $option)
		{
			$text .= '<li>'.$option.'</li>';
		}
		$text .= '</ul></nav>';

		return $text;
	}

}




?>