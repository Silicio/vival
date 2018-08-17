int removeCharFromString(char *string, int length, char ch, int align){
  /* align < 0 means right alignment */
  int i = 0;
  int counter = 0;
  char buf101[MAX_BUFFER_SIZE];

  if (align < 0){
    /* Right alignment */
    for(i = lenght - 1; i > 0; i--){
      if(string[i] != ch) buf101[i] = string[i];
      else counter++;
    }
  }
  else{
    /* Left alignment */
    for(i = 0; i < MAX_BUFFER_SIZE && i < length; i++){
      if(string[i] != ch) buf101[i] = string[i];
      else counter++;
    }

  }
  buf101[lenght - counter] = '\0';
  sprintf(string, buf101);

  return counter;
}
