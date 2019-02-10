## University of Bonn.
### IoT project for Semantic-Web. 
#### Department of Computer Science.
```
Winter semester 2019
```
---
### Arduino1
Based on [Arduino shield](https://store.arduino.cc/arduino-ethernet-shield-2)
Provides one resource to the network - the temperature value inside of the room.
The temperature sensor should be connected to analog input pin 0.

### *Query*
*Get temperature value:*
```
curl http://192.168.0.105/uni-bonn/raum1047/temperature
```
*Response*
```
@prefix ssn: <http://purl.oclc.org/NET/ssnx/ssn#>. 
@prefix cdt: <http://w3id.org/lindt/custom_datatypes#>. 
@prefix xsd:  <http://www.w3.org/2001/XMLSchema#>. 
@prefix sosa: <http://www.w3.org/ns/sosa/>.

	<> a sosa:Observation;
	sosa:hasSimpleResult "109 Cel"^^cdt:temperature.
```
*Response headers*
```
HTTP/1.1 200 OK
Content-Type: text/turtle
Content-Length: 280
Connection: close
```


---
### Arduino2
Based on [Arduino Mega-2560 shield](https://store.arduino.cc/arduino-mega-2560-rev3)
Provides one resource to the network - the external connected device state.
The external device should be connected to digital output pin 4.

### *Queries*
---
#### 1. *Get the device state:*  
*Query*
```
 curl http://192.168.0.106/uni-bonn/raum1047/led
 ```
*Response*
```
@prefix foaf: <http://xmlns.com/foaf/0.1/> .
@prefix saref: <https://w3id.org/saref#> .

	<> a saref:LightingDevice ;
	saref:hasState saref:Off .
```
*Response headers*
```
HTTP/1.1 200 OK
Content-Type: text/turtle
Content-Length: 144
Connection: close
```
---
#### 2.1 *Set the device state=Off:*  
*Query*
```
   curl -X PUT \
  http://192.168.0.106/uni-bonn/raum1047/led \
  -H 'Postman-Token: 8a7423cb-d7da-40df-878e-7b1bbb9f4aa4' \
  -H 'cache-control: no-cache' \
  -d '\@prefix foaf: <http://xmlns.com/foaf/0.1/> .
@prefix saref: <https://w3id.org/saref#> .

<> a saref:LightingDevice ; 
    saref:hasState saref:Off.'
 ```
*Response*
```
@prefix foaf: <http://xmlns.com/foaf/0.1/> .
@prefix saref: <https://w3id.org/saref#> .

	<> a saref:LightingDevice ;
	saref:hasState saref:Off .
```
*Response headers*
```
HTTP/1.1 200 OK
Content-Type: text/turtle
Content-Length: 146
Connection: close
```
---
#### 2.2 *Set the device state=On:*  
*Query*
```
   curl -X PUT \
  http://192.168.0.106/uni-bonn/raum1047/led \
  -H 'Postman-Token: 8a7423cb-d7da-40df-878e-7b1bbb9f4aa4' \
  -H 'cache-control: no-cache' \
  -d '\@prefix foaf: <http://xmlns.com/foaf/0.1/> .
@prefix saref: <https://w3id.org/saref#> .

<> a saref:LightingDevice ; 
    saref:hasState saref:On.'
 ```
*Response*
```
@prefix foaf: <http://xmlns.com/foaf/0.1/> .
@prefix saref: <https://w3id.org/saref#> .

	<> a saref:LightingDevice ;
	saref:hasState saref:On .
```
*Response headers*
```
HTTP/1.1 200 OK
Content-Type: text/turtle
Content-Length: 145
Connection: close
```
