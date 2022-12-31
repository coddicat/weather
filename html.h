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
      <header>
        <h3>Weather setting</h3>
      </header>
      <form>
        <fieldset id='weather-form'>
          <div class='inputs'>
            <label for='city' class='input'>
              <span>City:</span>
              <input type='text' id='city' name='city'>
            </label>
            <label for='country' class='input'>
              <span>Country:</span>
              <input type='text' id='country' name='country'>
            </label>
            <label for='apiKey' class='input'>
              <span>ApiKey(openweathermap.org):</span>
              <input type='text' id='apiKey' name='apiKey'>
            </label>
          </div>
          <div class='buttons'>
            <button id='save' type='button' onclick='saveHandler()'>Save</button>
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
          await fetch('/connect', {
            method: 'POST',
            headers: {
              'Content-Type': 'application/x-www-form-urlencoded'
            },
            body: `ssid=${ssid}&password=${password}`
          });
        } 
        catch(error) {
          alert(JSON.stringify(error));
        }
        finally {
          document.getElementById('wifi-form').disabled = false;
        }
      }

      async function disconnectHandler() {
        try {
          document.getElementById('wifi-form').disabled = true;
          const response = await fetch('/disconnect', {
            method: 'POST'
          });
        } finally {
          document.getElementById('wifi-form').disabled = false;
        }
      }

      async function saveHandler() {
        try {
          document.getElementById('weather-form').disabled = true;
          const city = document.getElementById('city').value;
          const country = document.getElementById('country').value;
          const apiKey = document.getElementById('apiKey').value;
          const response = await fetch('/save', {
            method: 'POST',
            headers: {
              'Content-Type': 'application/x-www-form-urlencoded'
            },
            body: `city=${city}&country=${country}&apiKey=${apiKey}`
          });
          const text = await response.text();
          alert(text);
        }
        catch(error) {
          alert(JSON.stringify(error));
        }
        finally {
          document.getElementById('weather-form').disabled = false;
        }
      }

      function handleStatus(status) {
        document.getElementById('status').innerHTML = status;
        document.getElementById('wifi-form').disabled = false;
        if (status.includes('CONNECTING')) {
          document.getElementById('wifi-form').disabled = true;
        } else if (status == 'CONNECTED') {
          document.getElementById('connect').disabled = true;
          document.getElementById('disconnect').disabled = false;
        } else {
          document.getElementById('connect').disabled = false;
          document.getElementById('disconnect').disabled = true;
        }
      }

      async function getStatus() {
        try {
          const response = await fetch('/status');
          const status = await response.text();
          handleStatus(status);
        } catch (error) {
          document.getElementById('status').innerHTML = 'N/A';
        } finally {
          setTimeout(getStatus, 2000);
        }
      }

      async function getData() {
        try {
          const response = await fetch('/data');
          const text = await response.text();
          const data = JSON.parse(text);
          const wifi = data['wifi'];
          const weather = data['weather'];
          const status = wifi['status'];
          const ssid = wifi['ssid'];
          const city = weather['city'];
          const country = weather['country'];
          const apiKey = weather['apiKey'];

          handleStatus(status);
          document.getElementById('ssid').value = ssid;
          document.getElementById('city').value = city;
          document.getElementById('country').value = country;
          document.getElementById('apiKey').value = apiKey;
        } catch (error) {
          document.getElementById('status').innerHTML = 'N/A';
        }
      }
      getData();
      setTimeout(getStatus, 2000);
    </script>
  </body>
</html>
)";

#endif
