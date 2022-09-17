void lamp_control(lampRele) {
  if (now.hour() >= 20 || now.hour() < 8) {
    digitalWrite(lampRele, HIGH);
  } else {
    digitalWrite(lampRele, LOW);
  }
}