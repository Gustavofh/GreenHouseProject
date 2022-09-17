void cooler_control(coolerRele) {
  if (now.minute() >= 5 && now.minute() < 10) {
    digitalWrite(coolerRele, HIGH);
  } else if (now.minute() >= 30 && now.minute() < 35) {
    digitalWrite(coolerRele, HIGH);
  } else {
    digitalWrite(coolerRele, LOW);
  }
}