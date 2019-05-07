<?php

defined('BASEPATH') OR exit('No direct script access allowed');

class Aieduscore extends CI_Controller 
{	

	
	function __construct()
	{
		parent::__construct();
		$this->load->helper('aieduscore');
		$this->load->helper('form');
	}


	function index()
	{

		/*
		$this->load->library('menu', array('a','b','c'));
		$data['menu'] = $this->menu->BuildMenu();
		$this->load->view('aieduscore/aieduscore_view', $data);
		*/

		$input_array = array($this->input->post('nombre'),$this->input->post('edad'));
	
		//TODO(Alex): transform them to numbers 
		$theta_array = array(		
		0.029613278844023802,
		0.23612139156180606,
		0.56204379562043794,
		0.10272596409574468,
		0.50000000000000000,
		1.0000000000000000,
		0.088848179528280280,
		0.00000000000000000,
		0.00000000000000000,
		1.0000000000000000,
		-32.824817097769838,
		-59.184547678625925,
		-94.551934035833440,
		-38.373512959363211,
		-96.247248604313711,
		-77.392233940881027,
		-118.24244192940937,
		-151.37200484487983,
		-212.14639317185714,
		-193.04458217675338,
		5.5508956723379703e-15,
		1.9791446159567591e-26,
		8.6420504038060089e-42,
		2.1607029780215679e-17,
		1.5861747522893243e-42,
		);
	

		$x_array = array(
	
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
	
		);
	
		$p = $this->calc_p($theta_array, $x_array);
		$p_p['data'] = $this->pretty_print_probability($p);

		$this->load->view('aieduscore/formulario', $p_p);
	}
	

	function calc_p($theta_array, $x_array)
	{
		
		assert(count($theta_array) == count($x_array));
		$result = 0;
		
		for($index = 0; 
		$index <  count($theta_array);	
		++$index)
		{

			$t = $theta_array[$index];
			$x = $x_array[$index];
		
			$mul_r = $x * $t;
			$result += $mul_r;

		}

		$p = (1.0 / (1.0 + exp(-$result))); 
		return $p;
	}
		
	function pretty_print_probability($p)
	{
		$tag_count = 5;
		$tag_array = array(
		'very unlikely to be a student',
		'unlikely to be a student',
		'more less probable to be a student',
		'likely to be a student',
		'very likely to be a student',

		);

		$exp_p = $p * $tag_count;
		$tag_index = floor($exp_p); 

		var_dump($tag_index);

		if($tag_index >= 5)
		{
			$tag_index = 4;
		}
		else if($tag_index < 0)
		{
			$tag_index = 0;
		}

		return $tag_array[$tag_index];
	}

	function load_form()
	{
		$this->load->view('aieduscore/formulario');
	}



	/*
	internal matrix_2    
	InnerProduct(matrix_2 * A, matrix_2 * B)
	{
	    Assert(A->CountY == B->CountX);
	    	
 	   matrix_2 Result = {};
 	   Result.CountX = A->CountX;
	    Result.CountY = B->CountY;
	    Result.Data = PushArray(&GetAIState()->TranArena, Result.CountX * Result.CountY, r64);
	    
 	   memory_index Count = MatrixCount(&Result);
  	  for(memory_index Index = 0;
        Index < Count;
	    ++Index)
	    {
	        r64 Product = 0;
  	      for(memory_index Y = 0;
    	        Y < A->CountY;
   	         ++Y)
     	   {
         	   //TODO(Alex): Make a faster linear combination.
          	  r64 AE = A->Data[(memory_index)(Index / Result.CountY) * A->CountY + Y];
           	 r64 BE = B->Data[(Y * B->CountY) + (Index % B->CountY)];
          	  
       	     Product += AE * BE;
       	 }
     	 	  
    	    Result.Data[Index] = Product;
    	}
   	 return Result;
	}
	*/


}


?>