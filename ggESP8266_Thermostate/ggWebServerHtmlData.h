#pragma once

char mWebServerHtmlFront[] PROGMEM = R"=====(
<!DOCTYPE html PUBLIC '-//W3C//DTD HTML 4.01//EN' 'http://www.w3.org/TR/html4/strict.dtd'>
<html>

  <head>
    <title>ESP8266 Thermostate</title>
    <meta http-equiv='Content-Type' content='text/html; charset=utf-8'/>
    <meta http-equiv='Content-Style-Type' content='text/css'/>
    <meta name='viewport' content='width=device-width,initial-scale=1,user-scalable=no'/>
    <link rel='shortcut icon' type='image/png' href='favicon.ico'/>
    <link rel='stylesheet' type='text/css' href='ggStyleSheet.css'/>
  </head>

  <body>
    <div class='cMainBlockDiv'>
)=====";

char mWebServerHtmlBack[] PROGMEM = R"=====(
      <hr noshade>
      <a href='/'>[home]</a>
      <a href='/loglive'>[log-live]</a>
      <a href='/logfile'>[log-file]</a>
      <a href='/files'>[files]</a>
      <a href='/debug'>[debug]</a>
      <a href='/goofy'>[goofy]</a><br>
      (c) 2023, <a href='http://www.laimer.ch' target='_blank'>Christoph Laimer</a>
    </div>
  </body>

</html>
)=====";
