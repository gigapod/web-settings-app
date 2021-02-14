
// App to connect to an embedded device (like SparkFun OpenLog) and manage settings via BLE


// SETUP - Define the Target BLE Service
//--------------------------------------------------------------------------------------
const kTargetServiceUUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
const kTargetServiceName = "SparkFun Board";
//--------------------------------------------------------------------------------------

console.clear();

//--------------------------------------------------------------------------------------
// Property Creation and Management
//
// Dynamically create and manage properties
//--------------------------------------------------------------------------------------

// BLE Codes for our service
const kBLEDescSFEPropCoreUUID = 0xA101;
const kBLEDescSFEPropRangeLimitsUUID = 0xA110;
const kBLEDescSFEGroupTitleUUID = 0xA112;

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
let deviceName=kTargetServiceName;
function setDeviceName(name){
    deviceName = name;
    document.getElementById("settings-title").innerHTML= name+ " Settings";
}
// DEBUG

let debugLoadTime=0;

// progress bar

function progress_set_value(value){

    document.getElementById("myBar").style.width=value + "%";

}
function progress_start(){

    progress_set_value(0);
    document.getElementById("myProgress").style.display="flex";
}
function progress_end(){
    document.getElementById("myProgress").style.display="none";    
}
//---------------------------------------
// property things
//---------------------------------------
// Used to main unique ids for property objects
let namecnt = 0;

const kBlkRange = 0x02;
const kBlkTitle = 0x01;

// Base property class 
class Property{

    constructor(bleChar, name, order){
      	this.characteristic = bleChar;
        this.name = name;
        this.order = order;
    	this.ID = "property-" + namecnt++;  // unique id for the DOM
        this.div = null;
        this.timer=-1;
        this.group=null;
        this.title=null;

    }
    
    processBlk(blkType, value, iPos){


        if(blkType != kBlkTitle){
            return iPos;
        }
        let len = value.getUint8(iPos++);
        this.title = dataToText(new DataView(value.buffer, iPos, len));
        return iPos + len;

    }
    init(){

        // For now - enable notifications on all values ...
        this.enableNotifications();
            
        // check for group title
        
        return new Promise( (resolve) => {

            // title?
            if(this.title != null && this.title.length > 2){
                this.group= document.createElement("div");

                this.group.innerHTML = ` <h2 class="title">`+ this.title + `</h2>`;  
                document.getElementById(targetID).appendChild(this.group);
            }
            this.generateElement();
            resolve(0);
            /*
            this.characteristic.getDescriptor(kBLEDescSFEGroupTitleUUID).then(descgrp => {
                 descgrp.readValue().then(value =>{
                    // decode name, set in instance data
                    let grpName= dataToText(value);
                    if(grpName.length > 2){
                        this.group= document.createElement("div");

                        this.group.innerHTML = ` <h2 class="title">`+ grpName + `</h2>`;  
                        document.getElementById(targetID).appendChild(this.group);
                    }
                    resolve(0);
                }).catch(error => {
                    console.log("Goup Title Value: ", error);
                    resolve(0);// keep moving on
                });
            }).catch(error => {
                // no title descp
                resolve(0);
            });
        
        // Once the name is resolved, if we have a name, generate the property UX.
        // By waiting, we ensure title rendered before the element            
        }).then(_ =>{
            this.generateElement();
            */
        }).catch(error => {
            console.log("Error setting up property: ", this.name);
        });
        

    }

    updateValue(value){}
    update(){
        this.characteristic.readValue().then( value =>{
            this.updateValue(value);
        });
    }
    enableNotifications(){
        if(this.characteristic){
            this.characteristic.startNotifications().then( _ =>{
                let target = this;
                this.characteristic.addEventListener('characteristicvaluechanged', function(event){
                    target.updateValue(event.target.value);
                });
            }).catch(error => {
                //console.log("Notifications not available.");
                // just eat this 
            });
        }
    }

    deleteElement(){

        if(this.div){
            this.div.remove();
            this.div=null;
        }
        if(this.group){
            this.group.remove();
            this.group=null;
        }

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
   		this.update();   			
   	}

    updateValue(value){
        this.inputField.checked = value.getUint8(0, true);
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

    processBlk(blkType, value, iPos){

        if(blkType != kBlkRange){
            return super.processBlk(blkType, value, iPos);
        }
        this.min = value.getInt32(iPos,true);
        this.max = value.getInt32(iPos+4, true);
        return iPos + 8;

    }
    
    _updateUX(value){
        //set text
        this.txtValue.setAttribute("data-length", ' ' + value);
        // set gutter color - 
        const percentage = (100 * (value - this.input.min)) / (this.input.max - this.input.min);
        const bg = `linear-gradient(90deg, ${range_fill} ${percentage}%, ${range_background} ${percentage + 0.1}%)`;
        this.input.style.background = bg;
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
            this._updateUX(event.target.value);
		});

   		// On change, set the underlying value
   		this.input.addEventListener("change", () => {
   			this.saveValue();
   		});
   		// and now update to the current value
   		this.update();   			
   	}

    updateValue(value){
        this.input.value = value.getInt32(0, true);
        this._updateUX(this.input.value);

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
   		this.update();   			
   	}

	updateValue(value){
        this.inputField.value = dataToText(value);
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
   		this.update();   			
   	}

    updateValue(value){
        // get the value from the BLE char and place it in the field
        this.inputField.value = value.getInt32(0, true);
    }

    saveValue(){
        // The arrows in the number field can fire off rapid events, which
        // can hammer the update, so buffer using a timer

        window.clearTimeout(this.timer);
        this.timer = window.setTimeout(()=>{
            // Get the value from the input field and save it to the characteristic
            let buff = new ArrayBuffer(4);
            let newValue = new Int32Array(buff);
            newValue[0] = this.inputField.value;
            this.characteristic.writeValue(buff);
        }, 1500);
        
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
        this.update();             
    }

    updateValue(value){
        // get the value from the BLE char and place it in the field
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
    }

    saveValue(){
        // Typing in the timefield is slow, so you can hammer the BLE
        // char with multiple sets as the date changes. Buffer this using a timer
        window.clearTimeout(this.timer);
        this.timer = window.setTimeout(()=>{
            // Get the value from the input field and save it to the characteristic
            this.characteristic.writeValue(textToData(this.inputField.value));
        }, 1500);

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
        this.update();             
    }

    updateValue(value){
        // Check for a valid time - split value, concat empty value to ensure 2 elements
        let digits = dataToText(value).split(":", 2).concat('');

        // if any of the time value are not a digit, default to current time
        if(isNaN(digits[0]) || isNaN(digits[1])){
            let currTime = new Date();                
            digits = [currTime.getHours(), currTime.getMinutes()];
        } 
        // set value - pad numbers with zeros
        this.inputField.value = [('0'+digits[0]).slice(-2), ('0'+digits[1]).slice(-2)].join(':');
    }

    saveValue(){
        // Typing in the timefield is slow, so you can hammer the BLE
        // char with multiple sets as the date changes. Buffer this using a timer

        window.clearTimeout(this.timer);
        this.timer = window.setTimeout(()=>{
            // Get the value from the input field and save it to the characteristic
            this.characteristic.writeValue(textToData(this.inputField.value));
        }, 1500);


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
      this.update();         
    }

    updateValue(value){
        // We are limiting floats to three decimal places.
        // Round out the floating number noise
        this.inputField.value = Math.round((value.getFloat32(0,true) + Number.EPSILON)*1000)/1000;
    }

    saveValue(){
        // The arrows in the number field can fire off rapid events, which
        // can hammer the update, so buffer using a timer

        window.clearTimeout(this.timer);
        this.timer = window.setTimeout(()=>{
            // Get the value from the input field and save it to the characteristic
            let buff = new ArrayBuffer(4);
            let newValue = new Float32Array(buff);
            newValue[0] = this.inputField.value;
            this.characteristic.writeValue(buff);
        }, 1500);
        
    }
}
// --------------------
// Delete all properties
function deleteProperties(){

    while(currentProperties.length > 0){
        currentProperties[0].deleteElement(); // delete UI
        currentProperties.splice(0,1); // pop out of array
    }
    //document.getElementById(targetID).style.display="none"; // hide settings area
}

function showProperties(){ 
/*
    if(currentProperties.length > 0){
       document.getElementById(targetID).style.display="flex"; 
    }
*/
}

function compairPropOrder(a, b){

    if ( a.order < b.order ){
        return -1;
    }
    if ( a.order > b.order ){
        return 1;
    }
    return 0;
}

async function renderProperties(){

    // first sort our props so they display as desired
    currentProperties.sort(compairPropOrder);

    progress_set_value(90);
    // build the UX for each property - want this is order -- so wait 
    for(const aProp of currentProperties){
        let result = await aProp.init();
    }
    progress_set_value(100);
    showProperties();
    endConnecting(true);
    // still found the UX isn't all there yet ... wait a cycle or 2
//    setTimeout(function(){showProperties();}, 100);
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
        // Get the core descriptor
        bleCharacteristic.getDescriptor(kBLEDescSFEPropCoreUUID).then(descType =>{

            if(descType == null){
                console.log("No type descriptor found.")
                resolve(-1);
            }
            //console.log(descs);
            // Get the value of the type descriptor
            descType.readValue().then(value =>{
                let iPos = 0;
                let attributes = value.getUint32(0,true);
                let order = attributes >> 8 & 0xFF;
                let type = attributes & 0xFF;

                if(type < kSFEPropTypeBool || type > propFactory.length ){
                    console.log("Invalid Type value: " + type);
                    resolve(1);
                }
                iPos += 4;
                // get name
                let lenName = value.getUint8(iPos++);
                let name = dataToText(new DataView(value.buffer, iPos, lenName));
                iPos += lenName;
                // build prop object - notice index into array of prop class defs 
                let property = new propFactory[type-1](bleCharacteristic, name, order);   

                // Does this property have onter data blocks in the descriptor value 
                while(iPos < value.byteLength){
                    let blkType = value.getUint8(iPos++);
                    iPos = property.processBlk(blkType, value , iPos);
                }
                currentProperties.push(property);

                resolve(0);
            }).catch(error => {
                console.log("readValue error: ", error);
                resolve(1);
            });
        }).catch(error => {
            console.log("getDescriptor - property type failed", error);
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

    document.getElementById("connect").innerHTML ="Connect To " + deviceName;

    // Delete our properties....
    deleteProperties();

    theGattServer=null;

}
function startConnecting(){ 
    document.body.style.cursor = "wait";
    // update button label
    let button= document.getElementById("connect");
    button.innerHTML ="Connecting to " + deviceName + '...';
    button.style.fontStyle='italic';
    button.disabled=true;
    button.style.cursor = "not-allowed";  

    progress_start();

}
function endConnecting(success){ 

    document.body.style.cursor = "default";
    let button= document.getElementById("connect");
    button.style.fontStyle ='';
    button.disabled=false;
    button.style.cursor = "auto";   
    progress_end();
    if(success){
        // update button label
        document.getElementById("connect").innerHTML ="Disconnect From " + deviceName;
    }else{
        onDisconnected();
    }
    console.log("Properties Load Time:", Date.now()-debugLoadTime);
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

        debugLoadTime=Date.now(); // time the loading of the settings parameters ...

        if(device.name)
            setDeviceName(device.name);

        progress_set_value(5);
        device.addEventListener('gattserverdisconnected', onDisconnected);

        return device.gatt.connect().then(gattServer => {
            
            bleConnected(gattServer);
            progress_set_value(25);            

            // Connect to our target Service 
            gattServer.getPrimaryService(kTargetServiceUUID).then(primaryService => {

                progress_set_value(60);
                // Now get all the characteristics for this service
                primaryService.getCharacteristics().then(theCharacteristics => {                

                    // Add the characteristics to the property sheet
                    // The adds are async - so use promises and then
                    // once everything is added, show the props UX all at once.
                    const promises=[];
                    let inc = 25./theCharacteristics.length;
                    let i = 60;
                    for(const aChar of theCharacteristics){
                        promises.push(addPropertyToSystem(aChar));
                        i += inc;
                        progress_set_value(i);
                    }
                    Promise.all(promises).then((results)=>{
                        renderProperties(); // build and display prop UX
                        
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
