import processing.serial.*;

Serial myPort;
PFont f;

int Angle = 0;
int Distance = 0;
float pixsDistance;

String portName = "COM11";   

void setup() {
  size(1250, 700);
  smooth();
  f = createFont("Arial Bold", 30);
  textFont(f);

  String[] ports = Serial.list();
  println("Available serial ports:");
  println(ports);

  boolean portFound = false;
  for (String p : ports) {
    if (p.equals(portName)) {
      portFound = true;
      break;
    }
  }

  if (!portFound && ports.length > 0) {
    println("Port " + portName + " not found. Auto-selecting: " + ports[0]);
    portName = ports[0];
  }

  if (ports.length > 0) {
    try {
      myPort = new Serial(this, portName, 115200); 
      myPort.bufferUntil('\n');
      println("Successfully connected to serial port: " + portName);
    } catch (Exception e) {
      println("Failed to open serial port " + portName + ": " + e.getMessage());
    }
  } else {
    println("WARNING: No serial ports detected. Connect Arduino and restart radar sketch.");
  }
}

void draw() {
  noStroke();
  fill(0, 25);
  rect(0, 0, width, height);

  fill(98, 245, 31);
  greenMesh();
  radarArea();
  infoText();
  greenLine();
  redLine();
}

// ================= SERIAL PARSER =================
void serialEvent(Serial port) {
  String line = port.readStringUntil('\n');
  if (line == null) return;

  line = trim(line);

  // Expected: "scan a=90 d=34"
  if (!line.startsWith("scan")) return;

  try {
    int aIndex = line.indexOf("a=");
    int dIndex = line.indexOf("d=");

    if (aIndex != -1 && dIndex != -1) {
      String aStr = line.substring(aIndex + 2, line.indexOf(" ", aIndex));
      String dStr = line.substring(dIndex + 2);

      Angle = int(aStr);
      Distance = int(dStr);

      // The Arduino sends d=0 when nothing is in range; without this, 0 drew a
      // full-length red "target" from the centre and read "Distance: 0 cm".
      if (Distance <= 0 || Distance > 40) Distance = 40;
    }
  } catch(Exception e) {
    println("Parser error: " + line);
  }
}

// ================= VISUAL ELEMENTS =================
void greenMesh() {
  pushMatrix();
  stroke(98, 245, 31);
  strokeWeight(2);
  translate(width / 2, height - height * 0.074);

  line(-width / 2, 0, width / 2, 0);

  for (int angle = 30; angle <= 150; angle += 30) {
    line(0, 0,
         (-width / 2) * cos(radians(angle)),
         (-width / 2) * sin(radians(angle)));
  }
  popMatrix();
}

void radarArea() {
  pushMatrix();
  translate(width / 2, height - height * 0.074);
  noFill();
  stroke(98, 245, 31);
  strokeWeight(2);

  arc(0, 0, (width - width * 0.0625), (width - width * 0.0625), PI, TWO_PI);
  arc(0, 0, (width - width * 0.27), (width - width * 0.27), PI, TWO_PI);
  arc(0, 0, (width - width * 0.479), (width - width * 0.479), PI, TWO_PI);
  arc(0, 0, (width - width * 0.687), (width - width * 0.687), PI, TWO_PI);
  popMatrix();
}

void greenLine() {
  pushMatrix();
  stroke(30, 250, 60);
  strokeWeight(9);
  translate(width / 2, height - height * 0.074);

  line(0, 0,
       (height - height * 0.12) * cos(radians(Angle)),
       -(height - height * 0.12) * sin(radians(Angle)));
  popMatrix();
}

void redLine() {
  pushMatrix();
  translate(width / 2, height - height * 0.074);
  strokeWeight(9);

  pixsDistance = Distance * ((height - height * 0.1666) * 0.025);

  if (Distance < 40) {
    stroke(255, 10, 10);
    line(pixsDistance * cos(radians(Angle)),
         -pixsDistance * sin(radians(Angle)),
         (width - width * 0.505) * cos(radians(Angle)),
         -(width - width * 0.505) * sin(radians(Angle)));
  }
  popMatrix();
}

void infoText() {
  fill(0);
  noStroke();
  rect(0, height - height * 0.0648, width, height);

  fill(98, 245, 31);
  textSize(25);

  text("10cm", width - width * 0.3854, height - height * 0.0833);
  text("20cm", width - width * 0.281, height - height * 0.0833);
  text("30cm", width - width * 0.177, height - height * 0.0833);
  text("40cm", width - width * 0.0729, height - height * 0.0833);

  textSize(30);
  text("Angle: " + Angle + " °", width - width * 0.48, height - height * 0.0277);

  if (Distance < 40) {
    text("Distance: " + Distance + " cm", width - width * 0.26, height - height * 0.0277);
  } else {
    text("Distance: Out of Range", width - width * 0.26, height - height * 0.0277);
  }
}
