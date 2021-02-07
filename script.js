
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
const kSFEPropTypeDate      = 0x5;
const kSFEPropTypeTime      = 0x6;
const kSFEPropTypeFloat     = 0x6;


var bIsConnected = false;
// Simple Data marshling code

const textDecoder = new TextDecoder();
function dataToText(data){ return textDecoder.decode(data);}

const textEncoder = new TextEncoder();
function textToData(text){ return textEncoder.encode(text);}

	
const targetID = 'settings-container';


const currentProperties=[];

function setDeviceName(name){
    document.getElementById("settings-title").innerHTML= name+ " Settings";
}

//---------------------------------------
// property things
//---------------------------------------
// Used to main unique ids for property objects
let namecnt = 0;

// Base property class 
class Property{

    constructor(bleChar){
      	this.characteristic = bleChar;
    	this.ID = "property-" + namecnt++;  // unique id for the DOM
        this.div = null;
    }

    init(){

        // get the name of this prop
        this.characteristic.getDescriptor(kBLEDescCharNameUUID).then(desc =>{

            desc.readValue().then(value =>{
                // decode name, set in instance data
                this.name = dataToText(value);

                // We have the name - call method to generate UX and complete setup
                this.generateElement();

            }).catch(error => {
                console.log(error);
            });
        }).catch(error => {
                console.log(error);
        });

    }

    generateElement(){} // stub

    deleteElement(){

        if(!this.div){
            return;
        }
        this.div.remove();
        this.div=null;
    }
}
// -------------------------------------------
class boolProperty extends Property{

   	generateElement(){

   	    this.div = document.createElement("div");

   		this.div.innerHTML = `
	   		<div class="setting">
				<input type="checkbox" id="`+ this.ID + `" />
				<label for="` + this.ID + `">` + this.name + `</label>
			</div>
   		`;

   		document.getElementById(targetID).appendChild(this.div);
   		this.inputField = this.div.querySelector('input');

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
        this.characteristic.getDescriptor(kBLEDescSFEPropRangeMinUUID).then(desc =>{
            desc.readValue().then(value =>{
                this.min = value.getInt32(0,true);

                this.characteristic.getDescriptor(kBLEDescSFEPropRangeMaxUUID).then(desc =>{
                    desc.readValue().then(value =>{
                        this.max = value.getInt32(0, true);

                        // Call super - finish setup
                        super.init();
                    });
                });
            });
        });

    }
    
    //------------------------
   	generateElement(){

   		this.div = document.createElement("div");
   		this.div.innerHTML = `
   			<div class="length range__slider" data-min="`+ this.min +`" data-max="` + this.max +`">
				<div class="length__title field-title" data-length='0'>`+ this.name +`:</div>
				<input id="slider" type="range" min="`+ this.min +`" max="`+ this.max +`" value="16" />
			</div>
   		`;
		this.input = this.div.querySelector('input');
		this.txtValue = this.div.querySelector('.length__title');

   		document.getElementById(targetID).appendChild(this.div);
   			
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

   		this.div = document.createElement("div");
   		this.div.innerHTML = `
	   		<div class="text-prop">
				<input type="text" id="`+ this.ID + `" />
				<label for="` + this.ID + `">` + this.name + `</label>
			</div>
   		`;
   		document.getElementById(targetID).appendChild(this.div);

   		this.inputField = this.div.querySelector('input');
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
class intProperty extends Property{

   	generateElement(){

   		this.div = document.createElement("div");

   		this.div.innerHTML = `
	   		<div class="number-prop">
        <input type="number" id="`+ this.ID + `" step="1" />
				<label for="` + this.ID + `">` + this.name + `</label>
			</div>
   		`;
   		document.getElementById(targetID).appendChild(this.div);

   		this.inputField = this.div.querySelector('input');
   		this.inputField.addEventListener("change", () => {
   			this.saveValue();
   		});
   		this.updateValue();   			
   	}

    updateValue(){
        // get the value from the BLE char and place it in the field
        this.characteristic.readValue().then( value =>{
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
//-------------------------------------------------------------
// dateProperty Object
class dateProperty extends Property{

    generateElement(){

        this.div = document.createElement("div");
        this.div.innerHTML = `
            <div class="date-prop">
                <input type="date" id="`+ this.ID + `" />
                <label for="` + this.ID + `">` + this.name + `</label>
            </div>
        `;
        document.getElementById(targetID).appendChild(this.div);

        this.inputField = this.div.querySelector('input');
        this.inputField.addEventListener("change", () => {
            this.saveValue();
        });
        this.updateValue();             
    }

    updateValue(){
        // get the value from the BLE char and place it in the field
        //console.log("Update Value Text");
        //this.inputField = "2021-02-02";

        // get the value from the BLE char and place it in the field
        this.characteristic.readValue().then( value =>{
           
            // Check for a valid date format...
            let digits = dataToText(value).split("-", 3).concat(['','']);

            // if any of the digits are not a digit, default to current date
            if(isNaN(digits[0]) || isNaN(digits[1]) || isNaN(digits[2])){
                let currTime = new Date();                
                digits = [currTime.getFullYear(), currTime.getMonth(), currTime.getDay()];
            } 
            if(digits[0].length == 2){ // short year value?
                digits[0] = '20'+digits[0];
            }
            // set value - pad month and day numbers with zeros
            this.inputField.value = [digits[0], ('0'+digits[1]).slice(-2), ('0'+digits[2]).slice(-2)].join('-');

        });
    }

    saveValue(){
        // Get the value from the input field and save it to the characteristic
        this.characteristic.writeValue(textToData(this.inputField.value));
    }
}

//-------------------------------------------------------------
// timeProperty Object
class timeProperty extends Property{

    generateElement(){

        this.div = document.createElement("div");
        this.div.innerHTML = `
            <div class="time-prop">
                <input type="time" id="`+ this.ID + `" />
                <label for="` + this.ID + `">` + this.name + `</label>
            </div>
        `;
        document.getElementById(targetID).appendChild(this.div);

        this.inputField = this.div.querySelector('input');
        this.inputField.addEventListener("change", () => {
            this.saveValue();
        });
        this.updateValue();             
    }

    updateValue(){
        // get the value from the BLE char and place it in the field
        //console.log("Update Value Text");
        //this.inputField = "HH:MM";

        // get the value from the BLE char and place it in the field
        this.characteristic.readValue().then( value =>{

            // Check for a valid time - split value, concat empty value to ensure 2 elements
            let digits = dataToText(value).split(":", 2).concat('');

            // if any of the time value are not a digit, default to current time
            if(isNaN(digits[0]) || isNaN(digits[1])){
                let currTime = new Date();                
                digits = [currTime.getHours(), currTime.getMinutes()];
            } 
            // set value - pad numbers with zeros
            this.inputField.value = [('0'+digits[0]).slice(-2), ('0'+digits[1]).slice(-2)].join(':');

        });
    }

    saveValue(){
        // Get the value from the input field and save it to the characteristic
        this.characteristic.writeValue(textToData(this.inputField.value));
    }
}
class floatProperty extends Property{

    generateElement(){

      this.div = document.createElement("div");
      this.div.innerHTML = `
        <div class="number-prop">
        <input type="number" id="`+ this.ID + `" step=".001" />
        <label for="` + this.ID + `">` + this.name + `</label>
      </div>
      `;
      document.getElementById(targetID).appendChild(this.div);

      this.inputField = this.div.querySelector('input');
      this.inputField.addEventListener("change", () => {
        this.saveValue();
      });
      this.updateValue();         
    }

    updateValue(){
        // get the value from the BLE char and place it in the field
        this.characteristic.readValue().then( value =>{
            // We are limiting floats to three decimal places.
            // Round out the floating number noise
            this.inputField.value = Math.round((value.getFloat32(0,true) + Number.EPSILON)*1000)/1000;
        });
    }

    saveValue(){
        // Get the value from the input field and save it to the characteristic
        let buff = new ArrayBuffer(4);
        let newValue = new Float32Array(buff);
        newValue[0] = this.inputField.value;
        this.characteristic.writeValue(buff);
    }
}
// --------------------
// Delete all properties
function deleteProperties(){

    while(currentProperties.length > 0){
        currentProperties[0].deleteElement(); // delete UI
        currentProperties.splice(0,1); // pop out of array
    }
    document.getElementById(targetID).style.display="none"; // hide settings area
}

function showProperties(){ 

    if(currentProperties.length > 0){
        document.getElementById(targetID).style.display="flex"; 
    }
}
//--------------------------------------------------------------------------------------
// Add a property to the system based on a BLE Characteristic
//
// Our property object defs = KEY: The order is same as type code index above. 
const propFactory = [boolProperty, intProperty, rangeProperty, textProperty, dateProperty, 
                     timeProperty, floatProperty];

// use a promise to encapsulate the async BLE calls
function addPropertyToSystem(bleCharacteristic){

    // use a promise to encapsualte this add - might be overkill ... but 
    // with a lot of props ...
    return new Promise( (resolve) => {
        // Get the type descriptor
        bleCharacteristic.getDescriptor(kBLEDescSFEPropTypeUUID).then(desc =>{

            // Get the value of the type descriptor
            desc.readValue().then(value =>{

                let type = value.getUint8(0,0);

                if(type < kSFEPropTypeBool || type > propFactory.length ){
                    console.log("Invalid Type value: " + type);
                    resolve(1);
                }
                // build prop object - notice index into array of prop class defs 
                let property = new propFactory[type-1](bleCharacteristic);   

                currentProperties.push(property);

                // init our property
                property.init();
                resolve(0);
            });
        }).catch(error => {
            console.log("getDescriptor - property type failed");
            console.log(error);
            resolve(1);
        });
    });
}

//--------------------------------------------------------------------------------------
// BLE Logic
//--------------------------------------------------------------------------------------

let theGattServer = null;

function disconnectBLEService(){


    if( theGattServer != null && theGattServer.connected){
        theGattServer.disconnect();

        theGattServer = null;
    }    

}
function bleConnected(gattServer){

    if(!gattServer || !gattServer.connected){
        return;
    }
    theGattServer = gattServer;


    
}
// disconnect event handler.
function onDisconnected(){

    document.getElementById("connect").innerHTML ="Connect To " + kTargetServiceName;

    // Delete our properties....
    deleteProperties();

    theGattServer=null;

}
function startConnecting(){ 
    document.body.style.cursor = "wait";
    // update button label
    let button= document.getElementById("connect");
    button.innerHTML ="Connecting to " + kTargetServiceName + '...';
    button.style.fontStyle='italic';
    button.disabled=true;
    button.style.cursor = "not-allowed";       

}
function endConnecting(success){ 

    document.body.style.cursor = "default";
    let button= document.getElementById("connect");
    button.style.fontStyle ='';
    button.disabled=false;
    button.style.cursor = "auto";   
    if(success){
        // update button label
        document.getElementById("connect").innerHTML ="Disconnect From " + kTargetServiceName;
    }else{
        onDisconnected();
    }
}
function connectToBLEService() {

    // is ble supported?
    if(typeof navigator.bluetooth == "undefined" ){
        alert(["Bluetooth is not available in this browser.", "Only Chrome-based browers support Bluetooth"]);
        return;
    }

    let filters = [];
    // KDB - for esp32 ...
    // Filtering on the name on it's own will find the device, but not the device with the
    // desired service. So adding the service to the search will find a device with the service
    // BUT not the desired name (at least in the returned objects ...)
    filters.push({name: kTargetServiceName}); 
    filters.push({services:[kTargetServiceUUID]});
    let options = {};
    options.filters = filters;
    startConnecting();
    return navigator.bluetooth.requestDevice(options).then(device => {
        
        if(device.name)
            setDeviceName(device.name);


        device.addEventListener('gattserverdisconnected', onDisconnected);

        return device.gatt.connect().then(gattServer => {
            
            bleConnected(gattServer);
            // Connect to our target Service 
            gattServer.getPrimaryService(kTargetServiceUUID).then(primaryService => {

                // Now get all the characteristics for this service
                primaryService.getCharacteristics().then(theCharacteristics => {                

                    // Add the characteristics to the property sheet
                    // The adds are async - so use promises and then
                    // once everything is added, show the props UX all at once.
                    const promises=[];
                    for(const aChar of theCharacteristics){
                        promises.push(addPropertyToSystem(aChar));
                    }
                    Promise.all(promises).then((results)=>{
                        // still found the UX isn't all there yet ... wait a cycle or 2
                        setTimeout(function(){showProperties();}, 300);
                        endConnecting(true);
                    });

                }).catch(error => {
                    console.log("getCharacteristics Error: " + error);
                    alert("Error communicating with device. Disconnecting.");
                    endConnecting(false);
                });


            }).catch(error => {
                console.log("getPrimaryService Error:" + error);
                alert("Unable to connect with the BLE settings service. Disconnecting.");
                endConnecting(false);
            });
        }).catch(error => {
            console.log(error);
            alert("Unable to connect with the BLE settings service. Disconnecting.");
            endConnecting(false);
        });
                
    }).catch(error => {
        console.log(error);
        alert("Unable to access with browsers BLE system. Disconnecting.");
        endConnecting(false);
    });

}
//--------------------------------------------------------------------------------------
// Misc Items
//--------------------------------------------------------------------------------------

// The connect button 
const connectBtn = document.getElementById("connect");
connectBtn.addEventListener("click", () => {
    if(theGattServer && theGattServer.connected){
        disconnectBLEService();
    }else{
        connectToBLEService();
    }

});
window.onload = function(){
    setDeviceName(kTargetServiceName);

}
