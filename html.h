#ifndef HTML_H
#define HTML_H

const String html = R"(
<html>
  <head>
    <title>Weather station settings</title>
    <meta name='viewport' content='user-scalable=no, initial-scale=1, maximum-scale=1, minimum-scale=1, width=device-width; viewport-fit=cover'>
    <style>
      .container { 
        max-width: 340px;
        margin: auto;
      }
      .inputs, .input {
        display: flex;
        flex-direction: column;
      }
      .inputs {
        gap: 1em;
        margin-bottom: 1.5em;
      }
      .input > input {
        font-size: 1.2em;
      }
      .buttons {
        display: flex;
        flex-direction: row;
        justify-content: end;
        gap: 1em;
      }
      .buttons > button {
        font-size: 1.2em;
      }
      body {
        font-size: 1em;
      }
    </style>
  </head>
  <body>
    <div class='container'>
      <header>
        <h1>Weather station settings</h1>
        <h3>Wifi status: <span id='status'></span></h3>
      </header>
      <form>
        <fieldset id='wifi-form'>
          <div class='inputs'>
            <label for='ssid' class='input'>
              <span>WiFi SSID:</span>
              <input type='text' id='ssid' name='ssid'>
            </label>
            <label for='password' class='input'>
              <span>WiFi Password:</span>
              <input type='password' id='password' name='password'>
            </label>
          </div>
          <div class='buttons'>
            <button id='connect' type='button' onclick='connectHandler()'>Connect</button>
            <button id='disconnect' onclick='disconnectHandler()'>Disconnect</button>
          </div>
        </fieldset>
      </form> 
    </div>
    
    <script async>
      async function connectHandler() {
        try {
          document.getElementById('wifi-form').disabled = true;

          const ssid = document.getElementById('ssid').value;
          const password = document.getElementById('password').value;

          const response = await fetch('/connect', {
            method: 'POST',
            headers: {
              'Content-Type': 'application/x-www-form-urlencoded'
            },
            body: `ssid=${ssid}&password=${password}`
          });
          // handle response
        } finally {
          //
        }
      }

      async function disconnectHandler() {
        try {
          document.getElementById('wifi-form').disabled = true;
          const response = await fetch('/disconnect', {
            method: 'POST'
          });
          // handle response
        } finally {
          //
        }
      }

      async function getStatus() {
        try {
          const response = await fetch('/status');
          const status = await response.text();

          document.getElementById('ssid').disabled = false;
          document.getElementById('password').disabled = false;
          document.getElementById('wifi-form').disabled = false;
          document.getElementById('connect').disabled = false;
          document.getElementById('disconnect').disabled = false;

          if (status.includes('CONNECTING')) {
            document.getElementById('status').innerHTML = status;
            document.getElementById('wifi-form').disabled = true;
          } else if (status.includes('CONNECTED')) {
            document.getElementById('ssid').disabled = true;
            document.getElementById('password').disabled = true;
            document.getElementById('connect').disabled = true;
            const ssid = status.substring('CONNECTED'.length + 1);
            document.getElementById('status').innerHTML = 'CONNECTED';
            document.getElementById('ssid').value = ssid;
          } else {
            document.getElementById('status').innerHTML = status;
            document.getElementById('disconnect').disabled = true;
          }
        } catch (error) {
          console.error(error);
          document.getElementById('status').innerHTML = 'N/A';
        } finally {
          setTimeout(getStatus, 1000);
        }
      }

      getStatus();

    </script>
  </body>
</html>


)";

#endif
