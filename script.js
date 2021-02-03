
// App to connect to an embedded device (like SparkFun OpenLog) and manage settings via BLE


// Define the Target BLE Service
//--------------------------------------------------------------------------------------
var uuidTargetService = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
//--------------------------------------------------------------------------------------

console.clear();
// set the body to full height
// document.body.style.height = `${innerHeight}px`


// function that handles the checkboxes state, so at least one needs to be selected. The last checkbox will be disabled.
function disableOnlyCheckbox(){
	let totalChecked = [uppercaseEl, lowercaseEl, numberEl, symbolEl].filter(el => el.checked)
	totalChecked.forEach(el => {
		if(totalChecked.length == 1){
			el.disabled = true;
		}else{
			el.disabled = false;
		}
	})
}
//--------------------------------------------------------------------------------------
// Property Creation and Management
//
// Dynamically create and manage properties
//--------------------------------------------------------------------------------------

// KDB HACKS
	const targetID = 'settings-container';
	const properties=[];

	let namecnt = 0;
	class Property{

    	constructor(bleChar){
          	this.characteristic = bleChar;
        	this.ID = "property-" + namecnt++;
    	}


    	getID(){return this.ID;}
	}
	// -------------------------------------------
  	class boolProperty extends Property{

   		generateElement(){


   			// GET title from the BLE characterisitic
   			//TODO
   			let name ="Testing Value";
			// TODO -- put most of this in the object
   			let div = document.createElement("div");
   			let ID = this.getID();
   			console.log(ID);
   			div.innerHTML = `
	   			<div class="setting">
					<input type="checkbox" id="`+ID + `" />
					<label for="` + ID + `">` + name + `</label>
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

	const range_fill = "#0B1EDF";
	const range_background = "rgba(255, 255, 255, 0.214)";   

  	class rangeProperty extends Property{

   		generateElement(){


   			// GET title from the BLE characterisitic
   			//TODO
   			let name ="Testing Value";
   			let min = 0;
   			let max = 200;

			// TODO -- put most of this in the object
   			let div = document.createElement("div");
   			let ID = this.getID();
   			console.log(ID);
   			div.innerHTML = `
   				<div class="length range__slider" data-min="`+min+`" data-max="`+max+`">
					<div class="length__title field-title" data-length='0'>`+name+`:</div>
					<input id="slider" type="range" min="`+min+`" max="`+max+`" value="16" />
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


   			// GET title from the BLE characterisitic
   			//TODO
   			let name ="A Text Value";
			// TODO -- put most of this in the object
   			let div = document.createElement("div");
   			let ID = this.getID();
   			console.log(ID);
   			div.innerHTML = `
	   			<div class="text-prop">
					<input type="text" id="`+ID + `" />
					<label for="` + ID + `">` + name + `</label>
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


   			// GET title from the BLE characterisitic
   			//TODO
   			let name ="A Integer Value";
			// TODO -- put most of this in the object
   			let div = document.createElement("div");
   			let ID = this.getID();
   			console.log(ID);
   			div.innerHTML = `
	   			<div class="text-prop">
					<input type="text" id="`+ID + `" onkeypress="return isNumberKey(event)"/>
					<label for="` + ID + `">` + name + `</label>
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

//--------------------------------------------------------------------------------------
// BLE Logic
//--------------------------------------------------------------------------------------
function connectToBLEService() {


  
    let filters = [];
    // KDB - for esp32 ...
    // Filtering on the name on it's own will find the device, but not the device with the
    // desired service. So adding the service to the search will find a device with the service
    // BUT not the desired name (at least in the returned objects ...)
    filters.push({name: "OpenLog"}); 
    filters.push({services:[uuidTargetService]});
    let options = {};
    options.filters = filters;

   

    return navigator.bluetooth.requestDevice(options).then(device => {
        
        console.log("Device Name: " + device.name);

        return device.gatt.connect().then(gattServer => {
            console.log("Connected to gattServer");
            console.log(gattServer);
            
            // Connect to our target Service 
            gattServer.getPrimaryService(uuidTargetService).then(primaryService => {
                console.log("Connected-> Primary Service");
                console.log(primaryService);

                // Now get all the characteristics for this service
                primaryService.getCharacteristics().then(theCharacteristics => {                

                    console.log("Service Characteristics");
                    for(const aChar of theCharacteristics){
                        console.log(aChar);
                        aChar.getDescriptor(0x2901).then(desc =>{
                            console.log("Retrieved Descriptor");
                            desc.readValue().then(value =>{
                                console.log(value);
                                let enc = new TextDecoder();
                                let name = enc.decode(value);

                                console.log(name);

                                // Lets get the type of this thing
                                aChar.getDescriptor(0xA101).then(desc => {
                                    desc.readValue().then(value =>{
                                        console.log(value);
                                        let type = value.getUint8(0,0);

                                        console.log("Char " + name + ", Type: " + type);

                                        if(type == 1){
                                            addBoolProperty();
                                        }
                                    });
                                });
                            })

                        });
                    }
                    console.log(theCharacteristics.map(c => c.uuid).join('\n' + ' '.repeat(10)));

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