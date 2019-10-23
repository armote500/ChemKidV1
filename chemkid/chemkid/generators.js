Blockly.JavaScript['chemkid.get_illuminance'] = function(block) {
	var dropdown_redled = block.getFieldValue('REDLED');
  var dropdown_greenled = block.getFieldValue('GREENLED');
  var dropdown_blueled = block.getFieldValue('BLUELED');
  var dropdown_gain = block.getFieldValue('GAIN');
  var dropdown_exposure_time = block.getFieldValue('EXPOSURE_TIME');
	var dropdown_output1 = block.getFieldValue('OUTPUT1');
	return [
		'DEV_I2C1.CHEMKID(' + block.getFieldValue('REDLED') + ', ' + block.getFieldValue('GREENLED') + ', ' + block.getFieldValue('BLUELED') + ', '
		+ block.getFieldValue('GAIN') + ', '
		+ block.getFieldValue('EXPOSURE_TIME') + ', '
		+ block.getFieldValue('OUTPUT1') + ').get_illuminance()',
		Blockly.JavaScript.ORDER_ATOMIC
	];
};
