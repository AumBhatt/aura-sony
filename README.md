## <p align="center"><img width="250" src="sony-logo-3068.png"/></p>
##### Latest Version : ./v5
### CLI Program Syntax <br>
```
./vX [IP-Address] [command]
```
Tested on Ubuntu 20.04 LTS.

`Note :` Any app fired using IRCC can wake-up the TV from Standby mode if Wake-On Lan is enabled.
### Wake-On LAN (WoL)
To check status of WoL : `./vX [IP-Address] wol check`<br>
To enable WoL : `./vX [IP-Address] wol true`<br>
To disable WoL : `./vX [IP-Address] wol false`<br>
### Commands
Power:
```
TvPower - for both ON/OFF.
```
Opening AV Input Menu:
```
Input
```
Open Home Menu:
```
Home
```
Arrow Keys:
```
Up   Down   Left   Right
```
Ok key:
```
Confirm
```
TV Key (Only on Sony TVs):
```
Tv
```
Back:
```
Return
```
Numeric keys:
```
NumX - replace X with 0-12
```
Volume:
```
VolumeUp   VolumeDown
```
Mute:
```
Mute - for both ON/OFF.
```

Similarly one can refer to `SonyIRCC.json` for all commands.

### Compiled using g++
```
g++ -o vX vX.cpp -lPocoNet -lPocoUtil -lPocoFoundation
```
### Headers for REST API
```
SOAPACTION: "urn:schemas-sony-com:service:IRCC:1#X_SendIRCC" // this part must be in quotes using '\' in C++
X-Auth-PSK: aura-sony
```
### XML Request Body
Type: `POST` <br>
Endpoint: `http://[IP-Address]/sony/ircc`
```
"<s:Envelope\
    xmlns:s='http://schemas.xmlsoap.org/soap/envelope/'\
    s:encodingStyle='http://schemas.xmlsoap.org/soap/encoding/'>\
    <s:Body>\
        <u:X_SendIRCC xmlns:u='urn:schemas-sony-com:service:IRCC:1'>\
            <IRCCCode>" + irccCode + "</IRCCCode>\
        </u:X_SendIRCC>\
    </s:Body>\
</s:Envelope>";
```
