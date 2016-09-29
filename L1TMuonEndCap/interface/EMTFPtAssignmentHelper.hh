namespace {

static const int GMT_eta_from_theta[128] = {
  239, 235, 233, 230, 227, 224, 222, 219, 217, 214, 212, 210, 207, 205, 203, 201,
  199, 197, 195, 193, 191, 189, 187, 186, 184, 182, 180, 179, 177, 176, 174, 172,
  171, 169, 168, 166, 165, 164, 162, 161, 160, 158, 157, 156, 154, 153, 152, 151,
  149, 148, 147, 146, 145, 143, 142, 141, 140, 139, 138, 137, 136, 135, 134, 133,
  132, 131, 130, 129, 128, 127, 126, 125, 124, 123, 122, 121, 120, 119, 118, 117,
  116, 116, 115, 114, 113, 112, 111, 110, 110, 109, 108, 107, 106, 106, 105, 104,
  103, 102, 102, 101, 100,  99,  99,  98,  97,  96,  96,  95,  94,  93,  93,  92,
   91,  91,  90,  89,  89,  88,  87,  87,  86,  85,  84,  84,  83,  83,  82,  81
};

static int getGMTEta(int theta, int endcap) {
  if (theta < 0)
    return 0;
  if (endcap == 2 && theta > 127)
    return -240;
  if (endcap == 1 && theta > 127)
    return 239;

  int eta = GMT_eta_from_theta[theta];
  if (endcap == 2)
    eta = -eta;
  return eta;
}

static int getGMTQuality(int mode, int theta) {
  int quality = 0;
  if (theta > 87) {  // if (eta < 1.2)
    switch (mode) {
    case 15:  quality = 8;  break;
    case 14:  quality = 4;  break;
    case 13:  quality = 4;  break;
    case 12:  quality = 4;  break;
    case 11:  quality = 4;  break;
    default:  quality = 4;  break;
    }

  } else {
    switch (mode) {
    case 15:  quality = 12; break;
    case 14:  quality = 12; break;
    case 13:  quality = 12; break;
    case 12:  quality = 8;  break;
    case 11:  quality = 12; break;
    case 10:  quality = 8;  break;
    case 7:   quality = 8;  break;
    default:  quality = 4;  break;
    }
  }
  quality |= (mode & 3);
  return quality;
}

static int getGMTCharge(int phi1, int phi2, int phi3, int phi4, int mode) {
  // -1 = postive physical charge to match pdgId code (i.e. -13 is positive, anti-muon). +1 = negative physical charge.
  // Also matches DN-2015/017 format for track finder --> uGMT interface format, where 0 indicates positive, 1 negative.
  int emuCharge = 0;
  int phidiffs[6] = {phi2 - phi1, phi3 - phi1, phi4 - phi1, phi3 - phi2, phi4 - phi2, phi4 - phi3};

  switch(mode) {
  case 15:  // 1-2-3-4
    if (phidiffs[0] > 0)                          // 1-2
      emuCharge = 1;
    else if (phidiffs[0] == 0 && phidiffs[1] < 0) // 1-3
      emuCharge = 1;
    else if (phidiffs[1] == 0 && phidiffs[2] < 0) // 1-4
      emuCharge = 1;
    else
      emuCharge = -1;
    break;

  case 14:  // 1-2-3
    if (phidiffs[0] < 0)                          // 1-2
      emuCharge = -1;
    else if (phidiffs[0] == 0 && phidiffs[1] < 0) // 1-3
      emuCharge = -1;
    else
      emuCharge = 1;
    break;

  case 13:  // 1-2-4
    if (phidiffs[0] > 0)                          // 1-2
      emuCharge = 1;
    else if (phidiffs[0] == 0 && phidiffs[2] < 0) // 1-4
      emuCharge = 1;
    else
      emuCharge = -1;
    break;

  case 12:  // 1-2
    if (phidiffs[0] > 0)                          // 1-2
      emuCharge = 1;
    else
      emuCharge = -1;
    break;

  case 11:  // 1-3-4
    if (phidiffs[1] > 0)                          // 1-3
      emuCharge = 1;
    else if (phidiffs[1] == 0 && phidiffs[2] < 0) // 1-4
      emuCharge = 1;
    else
      emuCharge = -1;
    break;

  case 10:  // 1-3
    if (phidiffs[1] > 0)                          // 1-3
      emuCharge = 1;
    else
      emuCharge = -1;
    break;

  case 9:   // 1-4
    if (phidiffs[2] > 0)                          // 1-4
      emuCharge = 1;
    else
      emuCharge = -1;
    break;

  case 7:   // 2-3-4
    if (phidiffs[3] > 0)                          // 2-3
      emuCharge = 1;
    else if (phidiffs[3] == 0 && phidiffs[4] < 0) // 2-4
      emuCharge = 1;
    else
      emuCharge = -1;
    break;

  case 6:   // 2-3
    if (phidiffs[3] > 0)                          // 2-3
      emuCharge = 1;
    else
      emuCharge = -1;
    break;

  case 5:   // 2-4
    if (phidiffs[4] > 0)                          // 2-4
      emuCharge = 1;
    else
      emuCharge = -1;
    break;

  case 3:   // 3-4
    if (phidiffs[5] > 0)                          // 3-4
      emuCharge = 1;
    else
      emuCharge = -1;
    break;

  default:
    emuCharge = -1;
    break;
  }

  int charge = 0;
  if (emuCharge == 1)
    charge = 1;
  return charge;
}

}  // namespace
