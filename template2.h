int get(unsigned short n){
  int l = 0;
  int r = sizeof(unicodes) / sizeof(unsigned short);
  while (l <= r)
  {
    int m = (l + r) / 2;
    if (unicodes[m] == n)
      return m;
    else if(unicodes[m] < n)
      l = m + 1;
    else
      r = m - 1;
  }
  return -1;
}