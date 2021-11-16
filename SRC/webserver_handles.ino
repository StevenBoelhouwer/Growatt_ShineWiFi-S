void handleRoot() {
  //if (httpServer.args() > 0) {
  //  handleGenericArgs();
  //}
  if (httpServer.hasArg("submit")) {
    // WIP
    httpServer.send(200, "text/html", index_page);
  }
  else {
    httpServer.send(200, "text/html", index_page);
  }
}

void handleSendJSON(void)
{
  httpServer.send(200, "application/json", mqttPayload);
}


void handleNotFound() {
  Serial.println("handleNotFound");
  String message = "Handle Not Found\n\n";
  message += "URI: ";
  message += httpServer.uri();
  message += "\nMethod: ";
  message += (httpServer.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += httpServer.args();
  message += "\n";
  for (uint8_t i = 0; i < httpServer.args(); i++) {
    message += " " + httpServer.argName(i) + ": " + httpServer.arg(i) + "\n";
  }
  httpServer.send(404, "text/plain", message);
}
