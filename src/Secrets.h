#include <pgmspace.h>
 
#define SECRET
 
const char WIFI_SSID[] = "LOOM SOLAR / BEE Solars Power";               //ZTE-b0aeaa //TAMIM2.4G // LOOM SOLAR / BEE Solars Power
const char WIFI_PASSWORD[] = "09791181639";           //0544287380
 // WiFi and MQTT credentials

//  const char WIFI_SSID[] = "ZTE-b0aeaa";               //ZTE-b0aeaa //TAMIM2.4G // LOOM SOLAR / BEE Solars Power
// const char WIFI_PASSWORD[] = "8ce117b0";      

const char *ssid = "ZTE-b0aeaa";
const char *password = "8ce117b0";
#define THINGNAME "TEST"
 
int8_t TIME_ZONE = -5; //NYC(USA): -5 UTC
 
//const char MQTT_HOST[] = "a3c3u73p81vtzu-ats.iot.us-east-1.amazonaws.com";
 
const char MQTT_HOST[] = "a3c3u73p81vtzu-ats.iot.ap-south-1.amazonaws.com";
 
static const char cacert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";
 
 
// Copy contents from XXXXXXXX-certificate.pem.crt here ▼
static const char client_cert[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
MIIDWTCCAkGgAwIBAgIUZomFSBj4H4ZIPwuFG1O73QULXhgwDQYJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI0MTAwODEyNDUx
MFoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAPN3WEKQkLDFf4ZQsi9/
EzVa2I6cbYX8Z0RIfLbzp4pJ4HRoqBZgyOT6mn/M1Hz7T2gEzusNkFXNqJc5XRs8
Cee+XeDnLqKiGyvjjysKBBpfGRFdYWWRJYbFL1+cthOfhzT8ntKttQ+N/b4m8Oo/
rTzU5/6ygn6mKFqsuE1TGwNxEoG7ZNNdScdKbzs9OIteQ+5vls6rPktglVtitCIe
mUtKSE2suaj6slzStSHDVB0l16tp66RjIJqIGjQheMTFvUuzDOJlQrWIM6suEidw
t094YUZl5+BRX6QoqLBVGw2/lPsR8u0UphXRABn2ABBR1WCB0QAalHftnMMb+5MD
e8cCAwEAAaNgMF4wHwYDVR0jBBgwFoAUz0Aul5E1R2WttoqH2A+PamNF+08wHQYD
VR0OBBYEFCBlWEvUT5RtL+rpjqTtAtzWCG3nMAwGA1UdEwEB/wQCMAAwDgYDVR0P
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQBzt5dZtw4MuXOcT+Tzg2YrR1ce
J88a1pgTyhmnUh6APr5Xs3gP/8SNTgzODWRHwSP9CV31+1XMIdfD4T4FUMfYytSY
OwjXwa5Kz7KQH9z7oOOC/ZBcYydxrdzdrCoCBqKA03zS9/7csAsdS/2qtaZg/tx8
SvzXGMaCQfW0wIT0NIxTzelB6nkOLHDMt00M/tD327o6Q0IwAoD4C3DshiHltCRN
l/C8KS0mwhQAhLs/uwnLrXDkaixDKAElRJuB6To/IGBKayxl9eYrvDBzksV5fB0W
V4ndMK3pwUiY4T+jFhlEYPN3nbekfkd6qMflyItB0LnrpEDrTPAYGAr9Y0I6
-----END CERTIFICATE-----



)KEY";
 
 
// Copy contents from  XXXXXXXX-private.pem.key here ▼
static const char privkey[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
MIIEpgIBAAKCAQEA83dYQpCQsMV/hlCyL38TNVrYjpxthfxnREh8tvOnikngdGio
FmDI5Pqaf8zUfPtPaATO6w2QVc2olzldGzwJ575d4OcuoqIbK+OPKwoEGl8ZEV1h
ZZElhsUvX5y2E5+HNPye0q21D439vibw6j+tPNTn/rKCfqYoWqy4TVMbA3ESgbtk
011Jx0pvOz04i15D7m+Wzqs+S2CVW2K0Ih6ZS0pITay5qPqyXNK1IcNUHSXXq2nr
pGMgmogaNCF4xMW9S7MM4mVCtYgzqy4SJ3C3T3hhRmXn4FFfpCiosFUbDb+U+xHy
7RSmFdEAGfYAEFHVYIHRABqUd+2cwxv7kwN7xwIDAQABAoIBAQDmCG/yzKZ47tGz
tVD2AWnDsqcD75anzrg/UwHK1mIJDg4tzqhqJF4Lwx/w+jYWNFhtXoWmnrUt7NeK
sl9fGRX+yOXoVmXAsctcZ3Bg6ODBjfKgbotUrJQ5Rtugye9akRLtpHXndsqo/K6v
WBmuKiXjYQIlqsITKajdK24jWQ8f6oEZqvcsd0aBxT4eCtHv+Xi3Sb3bU922ATDL
1f4pp1P61zXc0lstoYjcNNq/WxBBsAzeRXDx5HbUc8e9wxfSpOLvYB4HO4+H+4l6
sY67Idr0p/nn0AkfVC5NYZ6COEa1eL3XYBm/Lzu9qP8X/wkwlvRwRnymu6sd4KaP
eFaK9UjpAoGBAPvr0N+lBl3ZIYu/LNOFFFmFQyEDqtmD0ZWC03GxJgNkiKXYtsNp
Tq5btHQHOx4AeH8bynB/seqwLhbsJ0dvfzG0KA7Fmy9LknsnGUQkLjqK49rYEOOH
Yz0dl83me03bQdTu54UH93r/iI1/QLsg6bjuDY0Rir+pwHBtjwj6uDXFAoGBAPdo
e+dPnEuW7VFk0S92H74yC4aT9Fid8GI3pS8JTjmXDOi1/g+d6Q1rU008qElw3vJt
Ctsuhqppfs+5ZO39lphoPw92cX/att6FiEHoHqewJhRAMYyRIoBeKFLNwjL5+kQH
yrlOvLP3RMCsHwg8p83VGU+hi/l+uoPLeDGCa5AbAoGBAPFK6YGM2xBAzPTBY/ol
WVx1W9rfXGGC9NezuzsGZlaRkKDpdgStdQNkKu+HjYQrBz8YK0qdxJf5r+ucvfKa
RMYivMuJUh520BLNXFH8JZtFNQGEOoj0QIVlNuWsgSYNgHynZ3HwM4bIAs8kt6eE
2tiairg3uSHjQcNt7La5Dn/RAoGBANk/um1njsshgMrUSIvwofcVYuGN0iZuqcLV
MguXlEUlyoQeKzXzZ84MzF0qQmwa3OrrrXjB5iaF9IDbPzFsWG0EPTuKuieXGrVT
dE7LUP/WwdlOdTQFzdIC3E25uCmjMIvEe9M0vKcbHhhCDA3jlUY9NbijFpLDIWzR
on13FJS5AoGBAN2Jg3adcfke8JsgFBazqfZUnECditE0e17UjMZSnXPedhpJGgmi
nRRjN1dS7mHsplLNHzFXiRnA0kRqrK8FdiGAw3DD6DzP68SsNdSb9c6H+udMA67S
GPbzODPRyhT1ZyXwJjLEqExj1MWNdpohhkqJu9U4zT9uRxmx4XYSgPBT
-----END RSA PRIVATE KEY-----

)KEY";




// DigiCert High Assurance EV Root CA
const char trustRoot[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD
QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j
b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB
CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97
nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt
43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P
T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4
gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO
BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR
TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw
DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr
hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg
06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF
PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls
YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk
CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=
-----END CERTIFICATE-----
)EOF";