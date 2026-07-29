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

  println("Available ports:");
  println(Serial.list());

  myPort = new Serial(this, portName, 115200); 
  myPort.bufferUntil('\n');                    
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

    Angle = int(line.substring(aIndex + 2, dIndex).trim());
    Distance = int(line.substring(dIndex + 2).trim());

    Angle = constrain(Angle, 0, 180);
    Distance = constrain(Distance, 0, 400);

    println("Angle:", Angle, "Distance:", Distance);
  } 
  catch (Exception e) {
    println("Parse error:", line);
  }
}

// ================= RADAR DRAW =================
void radarArea() {
  pushMatrix();
  translate(width/2, height*0.98);
  noFill();
  strokeWeight(2);
  stroke(98, 245, 31);

  arc(0, 0, 1150, 1150, PI, TWO_PI);
  arc(0, 0, 850, 850, PI, TWO_PI);
  arc(0, 0, 550, 550, PI, TWO_PI);
  arc(0, 0, 250, 250, PI, TWO_PI);

  for (int i = 0; i <= 180; i += 30)
    line(0, 0, -600*cos(radians(i)), -600*sin(radians(i)));

  popMatrix();
}

void greenMesh() {
  stroke(98, 245, 31, 40);
  strokeWeight(1);
  for (int i = 0; i < height; i += 10) line(0, i, width, i);
  for (int i = 0; i < width; i += 10) line(i, 0, i, height);
}

void infoText() {
  fill(255);
  text("Auto Turret Radar", 20, 30);
  text("Angle : " + Angle + "°", 20, 60);
  text("Distance : " + Distance + " cm", 20, 90);
}

void greenLine() {
  pushMatrix();
  translate(width/2, height*0.98);
  strokeWeight(6);
  stroke(30, 250, 60);

  line(0, 0,
       600*cos(radians(Angle)),
      -600*sin(radians(Angle)));
  popMatrix();
}

void redLine() {
  if (Distance <= 0 || Distance > 80) return;

  pushMatrix();
  translate(width/2, height*0.98);
  strokeWeight(6);
  stroke(255, 0, 0);

  pixsDistance = map(Distance, 0, 80, 0, 600);

  float x = pixsDistance*cos(radians(Angle));
  float y = -pixsDistance*sin(radians(Angle));

  line(x, y,
       600*cos(radians(Angle)),
      -600*sin(radians(Angle)));
  popMatrix();
}