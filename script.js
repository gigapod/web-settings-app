
// App to connect to an embedded device (like SparkFun OpenLog) and manage settings via BLE


// SETUP - Define the Target BLE Service
//--------------------------------------------------------------------------------------
const kTargetServiceUUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
const kTargetServiceName = "OpenLog";
//--------------------------------------------------------------------------------------

console.clear();

//--------------------------------------------------------------------------------------
// Property Creation and Management
//
// Dynamically create and manage properties
//--------------------------------------------------------------------------------------

// BLE Codes for our service
const kBLEDescCharNameUUID = 0x2901;
const kBLEDescSFEPropTypeUUID = 0xA101;
const kBLEDescSFEPropRangeMinUUID = 0xA110;
const kBLEDescSFEPropRangeMaxUUID = 0xA111;

// Property type codes - sent as a value of the char descriptor 
const kSFEPropTypeBool      = 0x1;
const kSFEPropTypeInt       = 0x2;
const kSFEPropTypeRange     = 0x3;
const kSFEPropTypeText      = 0x4;


// Simple Data marshling code

const textDecoder = new TextDecoder();
function dataToText(data){ return textDecoder.decode(data);}

const textEncoder = new TextEncoder();
function textToData(text){ return textEncoder.encode(text);}
// KDB HACKS
	
const targetID = 'settings-container';
const properties=[];

function setDeviceName(name){
    document.getElementById("settings-title").innerHTML= name+ " Settings";
}
// Used to main unique ids for property objects
let namecnt = 0;

// Base property class 
class Property{

    constructor(bleChar){
      	this.characteristic = bleChar;
    	this.ID = "property-" + namecnt++;  // unique id for the DOM
    }

    init(){

        // get the name of this prop
        this.characteristic.getDescriptor(kBLEDescCharNameUUID).then(desc =>{

            desc.readValue().then(value =>{
                // decode name, set in instance data
                this.name = dataToText(value);

                // We have the name - call method to generate UX and complete setup
                this.generateElement();
// TODO ERROR HANDLING
            });
        });

    }

    generateElement(){} // stub
}
// -------------------------------------------
class boolProperty extends Property{

   	generateElement(){

   	    let div = document.createElement("div");

   		div.innerHTML = `
	   		<div class="setting">
				<input type="checkbox" id="`+ this.ID + `" />
				<label for="` + this.ID + `">` + this.name + `</label>
			</div>
   		`;

   		document.getElementById(targetID).appendChild(div);
   		this.inputField = div.querySelector('input');

        // event handler wireup - on change event, save the new value to BLE device
   		this.inputField.addEventListener("change", () => {
   			this.saveValue();
   		});
   		this.updateValue();   			
   	}

    updateValue(){

        // get the value from the BLE char and place it in the field
        this.characteristic.readValue().then( value =>{
            //console.log(`The  ${this.name} is: ${value.getUint8(0, true)}`);
            this.inputField.checked = value.getUint8(0, true);
        });
    }

    saveValue(){

        // Get the value from the input field and save it to the characteristic

        let buff = new ArrayBuffer(1);
        let newValue = new Uint8Array(buff);
        newValue[0] = this.inputField.checked;
        this.characteristic.writeValue(buff);
    }
}

//-------------------------------------------------------------
const range_fill = "#0B1EDF";
const range_background = "rgba(255, 255, 255, 0.214)";   

class rangeProperty extends Property{

    init(){
        // Get Min and Max of Range
        // Get the type descriptor

        this.characteristic.getDescriptor(kBLEDescSFEPropRangeMinUUID).then(desc =>{
            desc.readValue().then(value =>{

                this.min = value.getInt32(0,true);
                this.characteristic.getDescriptor(kBLEDescSFEPropRangeMaxUUID).then(desc =>{
                    desc.readValue().then(value =>{
                        this.max = value.getInt32(0, true);
                        super.init();
                    });
                });
            });
        });

    }
    
    //------------------------
   	generateElement(){

   		let div = document.createElement("div");
   		div.innerHTML = `
   			<div class="length range__slider" data-min="`+ this.min +`" data-max="` + this.max +`">
				<div class="length__title field-title" data-length='0'>`+ this.name +`:</div>
				<input id="slider" type="range" min="`+ this.min +`" max="`+ this.max +`" value="16" />
			</div>
   		`;
		this.input = div.querySelector('input');
		this.txtValue = div.querySelector('.length__title');

   		document.getElementById(targetID).appendChild(div);
   			
   		// Update slider values - event handler
   		this.input.addEventListener("input", event => {
   			//set text
			this.txtValue.setAttribute("data-length", ' '+event.target.value);
			// set gutter color - 
			const percentage = (100 * (this.input.value - this.input.min)) / (this.input.max - this.input.min);
			const bg = `linear-gradient(90deg, ${range_fill} ${percentage}%, ${range_background} ${percentage + 0.1}%)`;
			this.input.style.background = bg;
		});

   		// On change, set the underlying value
   		this.input.addEventListener("change", () => {
   			this.saveValue();
   		});
   		// and now update to the current value
   		this.updateValue();   			
   	}

   	updateValue(){
        // get the value from the BLE char and place it in the field
        this.characteristic.readValue().then( value =>{
            console.log(`The  ${this.name} is: ${value.getInt32(0, true)}`);
            this.input.value = value.getInt32(0, true);
           // send an event to the thing to trigger ui update
           this.input.dispatchEvent(new Event('input'));
        });


    }

    saveValue(){
        // Get the value from the input field and save it to the characteristic
        let buff = new ArrayBuffer(4);
        let newValue = new Int32Array(buff);
        newValue[0] = this.input.value;
        this.characteristic.writeValue(buff);
      }
}

//-------------------------------------------------------------
// textProperty Object
class textProperty extends Property{

   	generateElement(){

   		let div = document.createElement("div");
   		div.innerHTML = `
	   		<div class="text-prop">
				<input type="text" id="`+ this.ID + `" />
				<label for="` + this.ID + `">` + this.name + `</label>
			</div>
   		`;
   		document.getElementById(targetID).appendChild(div);

   		this.inputField = div.querySelector('input');
   		this.inputField.addEventListener("change", () => {
   			this.saveValue();
   		});
   		this.updateValue();   			
   	}

	updateValue(){
        // get the value from the BLE char and place it in the field
        //console.log("Update Value Text");

        // get the value from the BLE char and place it in the field
        this.characteristic.readValue().then( value =>{
           /// console.log(`The  ${this.name} is: ${value.getUint32(0, true)}`);
            this.inputField.value = dataToText(value);

        });
    }

    saveValue(){
        // Get the value from the input field and save it to the characteristic
        //console.log("Save Value Text: " + this.inputField.value);
        this.characteristic.writeValue(textToData(this.inputField.value));
    }
}

//-------------------------------------------------------------
// intProperty Object

// Function to limit text entry to numbers
   
function isNumberKey(evt){
    let charCode = (evt.which) ? evt.which : evt.keyCode
    if (charCode > 31 && (charCode < 48 || charCode > 57))
        return false;
    return true;
}
	
class intProperty extends Property{

   	generateElement(){

   		let div = document.createElement("div");
   		div.innerHTML = `
	   		<div class="number-prop">
				<input type="text" id="`+ this.ID + `" onkeypress="return isNumberKey(event)"/>
				<label for="` + this.ID + `">` + this.name + `</label>
			</div>
   		`;
   		document.getElementById(targetID).appendChild(div);

   		this.inputField = div.querySelector('input');
   		this.inputField.addEventListener("change", () => {
   			this.saveValue();
   		});
   		this.updateValue();   			
   	}

    updateValue(){
        // get the value from the BLE char and place it in the field
        this.characteristic.readValue().then( value =>{
            console.log(`The  ${this.name} is: ${value.getInt32(0, true)}`);
            this.inputField.value = value.getInt32(0, true);
        });
    }

    saveValue(){
        // Get the value from the input field and save it to the characteristic
        let buff = new ArrayBuffer(4);
        let newValue = new Int32Array(buff);
        newValue[0] = this.inputField.value;
        this.characteristic.writeValue(buff);
    }
}

//--------------------------------------------------------------------------------------
// Add a property to the system based on a BLE Characteristic
//
// Our property object defs = KEY: The order is same as type code index above. 
const propFactory = [boolProperty, intProperty, rangeProperty, textProperty];


function addPropertyToSystem(bleCharacteristic){

    // Get the type descriptor
    bleCharacteristic.getDescriptor(kBLEDescSFEPropTypeUUID).then(desc =>{

        // Get the value of the type descriptor
        desc.readValue().then(value =>{
            let type = value.getUint8(0,0);

            console.log("Descriptor Property Type: " + type);

            if(type < kSFEPropTypeBool || type > kSFEPropTypeText ){
                console.log("Invalid Type value: " + type);
                return;
            }
            // build prop object - notice index into array of prop class defs 
            let property = new propFactory[type-1](bleCharacteristic);            
            properties.push(property);
            // init our property
            property.init();

        });
    }).catch(error => {
        console.log("getDescriptor - property type failed");
        console.log(error);
    });
}
//--------------------------------------------------------------------------------------
// BLE Logic
//--------------------------------------------------------------------------------------
function connectToBLEService() {

    let filters = [];
    // KDB - for esp32 ...
    // Filtering on the name on it's own will find the device, but not the device with the
    // desired service. So adding the service to the search will find a device with the service
    // BUT not the desired name (at least in the returned objects ...)
    filters.push({name: kTargetServiceName}); 
    filters.push({services:[kTargetServiceUUID]});
    let options = {};
    options.filters = filters;

    return navigator.bluetooth.requestDevice(options).then(device => {
        
        if(device.name)
            setDeviceName(device.name);

        return device.gatt.connect().then(gattServer => {
            
            // Connect to our target Service 
            gattServer.getPrimaryService(kTargetServiceUUID).then(primaryService => {

                // Now get all the characteristics for this service
                primaryService.getCharacteristics().then(theCharacteristics => {                

                    // Add the characteristics to the property sheet
                    for(const aChar of theCharacteristics){
                        addPropertyToSystem(aChar);                        
                    }

                }).catch(error => {
                    console.log("getCharacteristics error");
                    console.log(error);
                });


            }).catch(error => {
                console.log("getPrimaryService error");
                console.log(error);
            });
        });            
                
    }).catch(error => {
        console.log(error);
    });

}
//--------------------------------------------------------------------------------------
// Misc Items
//--------------------------------------------------------------------------------------
  
// Wire up button events
let connectBtn = document.getElementById("connect")
connectBtn.addEventListener("click", () => {
    connectToBLEService();
});  
