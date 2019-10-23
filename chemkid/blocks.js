Blockly.Blocks['chemkid.get_illuminance'] = {
  init: function() {
		this.appendDummyInput()
        .appendField("ChemKid Education");
		this.appendDummyInput()
        .setAlign(Blockly.ALIGN_LEFT)
        .appendField("A) Light Source");
    this.appendDummyInput()
        .setAlign(Blockly.ALIGN_LEFT)
				.appendField("       Red")
        .appendField(new Blockly.FieldDropdown([["0","0"], ["8","8"], ["16","16"], ["32","32"], ["64","64"], ["128","128"], ["192","192"], ["255","255"]]), "REDLED")
        .appendField("    Green")
        .appendField(new Blockly.FieldDropdown([["0","0"], ["8","8"], ["16","16"], ["32","32"], ["64","64"], ["128","128"], ["192","192"], ["255","255"]]), "GREENLED")
        .appendField("    Blue")
        .appendField(new Blockly.FieldDropdown([["0","0"], ["8","8"], ["16","16"], ["32","32"], ["64","64"], ["128","128"], ["192","192"], ["255","255"]]), "BLUELED");
    this.appendDummyInput()
        .setAlign(Blockly.ALIGN_LEFT)
        .appendField("B) Sensor");
    this.appendDummyInput()
        .setAlign(Blockly.ALIGN_LEFT)
        .appendField("       Gain")
        .appendField(new Blockly.FieldDropdown([["x1","0x10"], ["x2","0x11"], ["x16","0x12"]]), "GAIN")
        .appendField("    Exposure")
        .appendField(new Blockly.FieldDropdown([["160 ms","0x00"], ["320 ms","0x01"], ["640 ms","0x02"]]), "EXPOSURE_TIME")
    this.appendDummyInput()
        .setAlign(Blockly.ALIGN_LEFT)
        .appendField("       ColorValue")
        .appendField(new Blockly.FieldDropdown([["Red","0"], ["Green","1"], ["Blue","2"], ["White", "3"]]), "OUTPUT1");
    this.setInputsInline(false);
		this.setOutput(true, 'Number');
    this.setColour(210);
 this.setTooltip("CHEMKID_GET_LUX_TOOLTIP");
 this.setHelpUrl("CHEMKID_GET_LUX_HELPURL");
  }
};
