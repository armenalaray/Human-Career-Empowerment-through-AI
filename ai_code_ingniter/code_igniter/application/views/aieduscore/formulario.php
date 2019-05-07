<!DOCTYPE html>
<html>
<head>
	<title>Formulario</title>
</head>
<body>

<?= form_open("/aieduscore/recibirdatos") ?>

<?php

	$nombre = array('name' => 'nombre', 'placeholder' => 'Escribe tu nombre');
	$edad = array('name' => 'edad', 'placeholder' => 'Escribe tu edad');

?>


<p>Formulario</p>


<?= form_label('Nombre:', 'nombre') ?>
<?= form_input($nombre) ?>

<?= form_label('Edad:', 'edad') ?>
<?= form_input($edad) ?>

<?= form_submit('', 'calcular probabilidad de entrada') ?>
<?= form_close() ?>

<?php

echo $data;

?>


</body>
</html>
