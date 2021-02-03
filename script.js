
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

// Property type codes - sent as a value of the char descriptor 
const kSFEPropTypeBool      = 0x1;
const kSFEPropTypeInt       = 0x2;
const kSFEPropTypeRange     = 0x3;
const kSFEPropTypeText      = 0x4;


// Simple Data marshling code

const textDecoder = new TextDecoder();
function dataToText(data){ return textDecoder.decode(data);}

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
           console.log("Update Value Bool");
           this.inputField.checked= true;
    	}

       	saveValue(){
           // Get the value from the input field and save it to the characteristic
           console.log("Save Value Bool: " + this.inputField.checked);
      	}
   }

	function addBoolProperty(){

  
   		let char = "dummy"; // image this being a ble characteristic
   		
		let newProperty = new boolProperty(char);
   		properties.push(newProperty);
   		newProperty.generateElement(); // generate the HTML
   }

   //-------------------------------------------------------------
	const range_fill = "#0B1EDF";
	const range_background = "rgba(255, 255, 255, 0.214)";   

  	class rangeProperty extends Property{

        int(){
            // Get Min and Max of Range
            //TODO

            this.min = 0;
            this.max = 200;

            // call super to finish setup
            super.init();            
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
           console.log("Update Value Range");
           this.input.value=10;  // TODO
           // send an event to the thing to trigger ui update
           this.input.dispatchEvent(new Event('input'));
    	}

       	saveValue(){
           // Get the value from the input field and save it to the characteristic
           console.log("Save Value Range: " + this.input.value);
      	}
   }

	function addRangeProperty(){
  
   		let char = "dummy"; // image this being a ble characteristic
   		
		let newProperty = new rangeProperty(char);
   		properties.push(newProperty);
   		newProperty.generateElement(); // generate the HTML
   }

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
           console.log("Update Value Text");
           this.inputField.value="Some Text";
    	}

       	saveValue(){
           // Get the value from the input field and save it to the characteristic
           console.log("Save Value Text: " + this.inputField.value);
      	}
   }

	function addTextProperty(){

  
   		let char = "dummy"; // image this being a ble characteristic
   		
		let newProperty = new textProperty(char);
   		properties.push(newProperty);
   		newProperty.generateElement(); // generate the HTML
   }

   function isNumberKey(evt){
    var charCode = (evt.which) ? evt.which : evt.keyCode
    if (charCode > 31 && (charCode < 48 || charCode > 57))
        return false;
    return true;
}
	class intProperty extends Property{

   		generateElement(){

   			let div = document.createElement("div");
   			div.innerHTML = `
	   			<div class="text-prop">
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
           console.log("Update Value Number");
           this.inputField.value=134;
    	}

       	saveValue(){
           // Get the value from the input field and save it to the characteristic
           console.log("Save Value Number: " + this.inputField.value);
      	}
   }

	function addIntProperty(){

  
   		let char = "dummy"; // image this being a ble characteristic
   		
		let newProperty = new intProperty(char);
   		properties.push(newProperty);
   		newProperty.generateElement(); // generate the HTML
   }

// Our property object defs = KEY: The order is same as type code index above. 
const propFactory = [boolProperty, intProperty, rangeProperty, textProperty];

//--------------------------------------------------------------------------------------
// Add a property to the system based on a BLE Characteristic
//
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
            console.log("Connected to gattServer");
            console.log(gattServer);
            
            // Connect to our target Service 
            gattServer.getPrimaryService(kTargetServiceUUID).then(primaryService => {
                console.log("Connected-> Primary Service");
                console.log(primaryService);

                // Now get all the characteristics for this service
                primaryService.getCharacteristics().then(theCharacteristics => {                

                    console.log("Service Characteristics");

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

  const toggleBtn = document.getElementById("add-toggle")
 toggleBtn.addEventListener("click", () => {
   addBoolProperty();
 });
// When Generate is clicked Password id generated.
  const rangeBtn = document.getElementById("add-range")
rangeBtn.addEventListener("click", () => {
	addRangeProperty();
});
const textBtn = document.getElementById("add-text")
textBtn.addEventListener("click", () => {
	addTextProperty();
});
const numBtn = document.getElementById("add-number")
numBtn.addEventListener("click", () => {
	addIntProperty();
});