void printPage(){
    #ifdef ESP32   // начинаем прослушивать входящих клиентов:

  WiFiClient client = server.available();
  if (client) {   
    String currentLine = "";        
    while (client.connected()) {    
      if (client.available()) {     
        char c = client.read();      
        header += c;
        if (c == '\n') {            
          if (currentLine.length() == 0) {
           
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // показываем веб-страницу с помощью этого HTML-кода:
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<meta http-equiv=\"Refresh\" content=\"2\" />");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println("</style></head>");
            client.println("<body><h1>CG-ANEM</h1><br>");
            client.println("<p>WIND SPEED &#128168: " + String(cganem.airflowRate) + "</p>");
            client.println("<p>WIND TEMP &#127777: " + String(cganem.temperature) + "</p>");
            client.println("</body></html>");
            client.println();
            break;
          } else {  
            currentLine = "";
          }
        } else if (c != '\r') { 
          currentLine += c;      
        }
      }
    }
    header = "";
    client.stop();
  }
  #endif
}