bool toggle_on = false;
bool toggle_off = false;

void actuate(uint8_t act_out)
{
  if (toggle_on)
  {
    digitalWrite(act_out, HIGH);
    toggle_on = false;
  }
  else if (toggle_off)
  {
    digitalWrite(act_out, LOW);
    toggle_off = false;
  }
  //delay(1);
}