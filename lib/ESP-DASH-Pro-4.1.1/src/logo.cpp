#include "logo.h"

/*
    This file contains the logo which is used in ESP-DASH Pro.
    The logo is an image which is converted to a C array using the following utility:
    1. Go to https://ayushsharma82.github.io/file2raw/
    2. Upload your image file
    3. Select "Gzip Compress" & "Use PROGMEM (Arduino)"
    4. Click on "Convert" button
    5. Copy the generated data and replace the existing value of logo in this file
    6. Set correct mime type according to your image file and adjust the width and height of the image as desired.
*/

const uint8_t DASH_MINI_LOGO[] PROGMEM = {
    31,139,8,0,0,0,0,0,2,3,53,78,213,145,196,48,12,109,69,163,127,203,20,92,222,173,228,40,19,102,
    78,245,39,31,24,69,15,46,227,146,194,146,12,99,222,54,87,212,100,16,182,186,106,198,43,102,211,212,157,164,
    92,215,149,86,75,237,144,74,163,148,146,60,143,176,228,201,250,106,183,43,42,80,16,251,160,149,143,176,230,95,
    83,118,197,152,195,44,201,211,108,186,162,171,223,46,227,180,87,201,237,34,255,254,238,125,202,0,190,174,88,43,
    242,65,101,158,37,191,55,154,34,230,178,1,89,208,62,25,208,30,133,28,253,254,54,228,178,2,99,200,138,255,
    130,239,34,159,71,196,31,40,19,142,233,168,173,230,114,232,83,156,105,211,199,164,65,185,161,128,60,8,248,13,
    249,213,33,40,161,21,147,113,28,80,36,120,152,91,156,248,238,203,132,54,7,74,231,120,73,111,223,179,87,21,
    105,34,1,0,0
};

const uint8_t DASH_LARGE_LOGO[] PROGMEM = {
    31,139,8,0,0,0,0,0,2,3,0,194,1,61,254,137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,
    82,0,0,2,14,0,0,0,90,8,6,0,0,0,208,59,222,241,0,0,0,1,115,82,71,66,1,217,201,44,
    127,0,0,0,9,112,72,89,115,0,0,11,19,0,0,11,19,1,0,154,156,24,0,0,29,201,73,68,65,84,
    120,156,237,157,63,236,37,73,113,199,71,119,11,104,35,31,198,201,9,108,89,14,17,194,233,90,224,152,224,228,
    208,66,186,77,142,128,4,100,89,178,100,153,192,96,33,203,58,91,190,187,200,18,18,22,136,144,216,36,96,140,
    232,204,70,231,128,100,109,17,152,232,88,193,157,45,214,39,7,200,146,111,253,251,206,189,122,91,211,211,255,167,
    171,187,167,95,125,165,210,238,239,253,254,76,79,207,188,169,207,171,170,174,94,22,149,234,54,244,226,157,61,188,
    179,87,47,102,46,246,196,97,134,217,171,151,223,123,208,116,180,170,169,244,225,239,61,249,181,144,221,255,194,227,
    23,62,244,251,111,254,174,132,225,111,115,163,99,246,158,19,213,249,245,226,96,166,218,75,231,63,95,4,10,128,
    129,167,21,140,160,2,127,179,231,60,157,245,186,159,117,220,135,5,71,253,220,143,223,125,10,123,254,251,191,220,
    217,189,47,191,253,244,222,31,252,219,213,62,248,123,255,242,127,57,102,255,46,255,151,236,249,207,189,181,218,189,
    175,255,215,106,31,251,225,147,71,100,47,252,232,191,255,226,215,255,238,63,95,2,88,244,158,43,213,57,132,79,
    82,53,30,170,18,198,63,1,210,39,69,140,247,20,15,139,138,130,3,108,53,231,166,205,41,137,9,247,71,77,
    88,136,205,21,174,77,203,251,81,234,253,106,26,140,93,226,154,60,105,48,238,195,2,56,16,36,56,1,226,143,
    127,254,148,67,64,41,52,248,224,99,3,16,119,199,34,120,248,237,239,62,121,143,236,195,223,120,231,125,128,249,
    252,227,95,220,123,229,173,215,1,18,26,153,80,249,52,45,119,113,32,83,255,247,254,182,141,173,225,66,83,209,
    37,67,101,32,21,220,16,24,0,97,0,0,21,16,46,64,239,208,182,97,162,64,162,36,128,160,183,82,129,84,
    108,40,42,130,64,222,170,171,42,129,40,74,21,250,173,207,247,119,255,127,157,115,206,185,207,121,206,243,28,130,
    122,164,99,72,166,37,94,1,193,58,173,129,184,195,211,189,1,115,209,7,186,78,55,103,24,41,64,139,179,201,
    9,220,109,247,101,133,66,155,102,158,51,162,97,133,115,154,118,234,126,23,20,32,142,28,233,52,103,23,164,203,
    73,126,151,189,117,124,179,176,66,148,40,162,232,17,200,135,159,96,157,121,172,171,94,108,191,240,51,177,249,201,
    149,49,209,143,189,202,155,203,134,122,241,121,168,114,164,241,236,178,240,1,58,246,200,252,181,87,73,79,217,114,
    250,24,0,135,163,136,178,7,6,229,186,16,58,158,142,229,98,189,29,232,118,47,139,208,224,34,30,177,182,79,
    47,8,206,40,56,86,149,166,166,165,66,19,219,251,220,122,94,218,17,158,28,252,182,226,206,157,121,91,53,113,
    246,10,121,202,142,8,116,168,167,110,23,3,111,137,234,138,200,192,9,63,214,66,245,95,113,186,247,108,173,222,
    15,197,97,102,71,206,232,57,109,106,236,134,154,18,18,90,243,221,126,245,191,249,52,71,188,187,214,217,235,159,
    118,112,188,196,140,143,189,249,59,49,193,139,203,197,10,117,172,25,217,36,16,140,30,241,73,123,162,12,249,97,
    250,76,236,5,189,218,0,62,252,150,129,110,40,177,210,172,136,121,106,52,230,81,80,221,57,82,175,243,234,150,
    3,175,136,229,149,59,92,6,150,37,44,98,132,172,60,41,124,147,72,204,52,100,8,40,39,14,227,51,210,97,
    186,140,157,204,59,159,208,181,153,22,13,29,40,231,155,183,145,75,133,52,175,145,44,121,151,182,100,185,235,251,
    252,157,15,255,170,70,26,75,255,155,121,196,37,243,73,122,90,41,158,254,34,87,70,214,152,18,81,202,123,32,
    233,236,225,249,56,200,76,194,87,87,146,59,99,52,51,163,25,171,132,24,8,95,57,108,178,167,127,36,190,195,
    20,211,112,215,8,219,42,185,82,198,27,165,173,240,173,88,39,62,58,151,153,182,11,58,85,188,241,97,253,248,
    141,125,70,8,226,113,142,251,103,75,92,230,223,79,44,1,201,62,146,116,184,109,43,14,88,43,59,66,93,162,
    14,170,243,189,52,255,59,128,210,108,201,184,181,215,188,54,58,75,217,133,66,128,137,149,202,81,196,112,135,165,
    87,138,179,255,228,217,179,46,153,145,110,155,134,141,190,84,86,87,119,216,252,56,135,113,74,190,230,19,109,80,
    221,196,42,48,79,150,247,56,179,40,78,126,158,215,64,115,48,215,111,162,148,49,97,158,128,144,247,207,27,156,
    237,198,3,234,227,54,146,44,116,202,179,38,223,175,113,49,55,144,161,121,63,55,69,222,139,61,174,203,49,143,
    8,40,167,255,144,69,203,89,39,174,159,126,97,247,2,17,40,17,39,89,204,225,62,62,216,38,221,232,121,136,
    79,150,246,219,160,141,195,245,110,183,56,222,88,206,40,8,85,212,157,78,11,151,0,221,143,170,69,107,208,244,
    58,3,79,4,84,157,222,245,161,131,87,244,212,28,135,59,203,209,255,253,37,66,127,47,117,138,177,147,26,14,
    106,69,128,143,42,48,102,51,220,44,237,182,17,217,141,223,154,234,57,104,233,127,6,91,45,189,91,1,160,65,
    186,232,136,228,156,67,75,238,209,177,50,239,111,231,131,119,40,49,14,241,177,27,174,97,192,70,171,106,175,73,
    243,28,78,231,214,69,225,205,165,16,156,176,99,107,163,213,222,70,83,218,52,87,14,19,9,205,251,142,57,156,
    66,30,73,178,132,165,135,102,203,215,139,111,156,168,70,236,182,68,10,243,220,126,13,226,91,165,231,50,174,84,
    10,77,71,45,72,145,192,150,56,73,152,190,186,215,21,161,35,88,246,173,228,143,58,203,95,152,102,251,117,27,
    155,82,149,140,68,167,58,101,219,153,221,222,177,51,174,167,185,11,82,16,20,244,176,123,131,6,39,4,84,114,
    186,131,128,187,189,138,249,47,43,133,78,236,139,246,254,191,48,136,47,7,205,64,24,103,247,12,113,28,118,24,
    194,48,167,91,245,81,174,104,44,172,58,22,67,167,42,159,44,183,103,223,251,59,251,215,86,26,102,150,14,77,
    49,138,45,166,172,26,251,207,71,254,54,68,4,81,24,213,4,250,187,22,63,102,255,21,35,216,68,170,86,147,
    59,187,38,151,203,150,135,27,93,226,61,91,62,181,148,207,76,169,91,86,2,47,44,185,126,127,100,230,95,202,
    63,133,30,171,51,155,173,93,166,167,142,6,243,95,190,90,124,48,127,62,128,59,163,180,103,115,106,63,138,175,
    214,176,113,134,81,115,96,235,23,246,214,67,179,96,226,157,161,59,214,0,112,181,8,111,91,79,174,110,165,90,
    142,114,63,127,243,141,191,50,90,219,189,209,222,204,94,83,16,182,47,53,221,172,122,117,72,44,90,41,152,24,
    94,215,88,207,171,242,5,29,75,167,183,220,136,171,86,65,9,253,249,161,115,182,147,145,54,161,191,206,26,200,
    179,10,172,220,167,116,114,6,71,24,156,207,204,7,242,205,56,134,149,192,170,37,151,27,118,116,94,202,212,154,
    215,151,178,188,156,218,150,189,74,235,197,58,102,119,242,86,218,206,130,227,93,247,244,254,108,250,220,60,54,158,
    165,45,162,128,2,33,221,73,4,246,18,11,94,143,93,9,155,24,80,105,60,181,74,128,86,220,241,59,161,32,
    179,39,71,170,17,164,117,156,34,224,16,60,77,27,87,244,34,75,197,114,223,224,119,233,193,197,47,50,11,39,
    190,36,189,240,21,188,71,227,237,240,187,149,174,154,207,179,201,27,159,29,238,216,118,126,119,8,204,103,108,9,
    251,162,60,237,82,152,8,215,109,86,36,67,55,113,235,203,129,182,159,60,116,123,205,76,73,23,87,0,208,1,
    27,151,233,52,18,248,75,207,250,158,169,141,67,78,179,185,139,249,242,176,95,129,46,234,159,198,81,124,250,244,
    193,201,119,71,22,30,38,223,69,232,182,175,52,33,148,36,1,244,164,16,49,248,141,69,113,233,132,232,235,58,
    191,17,159,135,155,149,211,183,167,255,94,149,241,15,229,183,30,124,107,127,248,86,182,238,57,119,123,36,242,236,
    112,88,66,206,101,7,39,31,11,38,54,169,94,233,54,251,217,93,219,164,163,82,33,55,166,183,250,120,228,241,
    220,228,107,178,238,161,100,207,210,202,245,114,183,44,179,219,127,193,66,132,13,46,96,139,21,203,208,89,36,95,
    36,83,125,168,19,197,127,72,250,180,12,93,129,15,175,106,208,179,89,40,45,177,83,161,138,201,82,58,137,126,
    55,140,111,136,86,152,24,212,91,169,126,35,232,118,43,210,66,175,54,84,219,139,184,167,53,45,221,89,120,223,
    139,79,137,89,69,49,237,70,98,101,89,89,15,155,40,78,3,82,110,226,224,175,7,250,194,231,56,12,144,130,
    102,222,186,201,99,186,237,160,42,176,161,0,25,208,241,25,122,147,117,231,139,47,137,227,14,199,148,128,137,201,
    43,183,244,207,214,60,214,181,36,194,172,62,17,137,143,85,143,55,174,19,215,9,47,223,157,57,251,125,54,187,
    244,207,72,231,159,193,85,159,228,144,255,6,75,126,191,184,54,19,56,241,230,222,93,103,131,200,29,48,82,255,
    101,46,34,101,21,145,60,106,51,28,162,90,197,225,58,74,250,168,234,62,104,31,77,85,249,11,102,126,240,211,
    246,4,130,164,231,48,63,116,146,131,1,239,120,173,185,62,187,221,176,2,215,49,210,191,69,174,132,117,14,248,
    224,21,56,10,191,68,176,120,40,100,212,104,244,206,40,211,154,201,215,47,55,198,30,186,60,247,32,164,255,150,
    45,142,232,179,209,183,251,141,209,192,51,253,233,223,207,254,164,162,249,61,135,68,176,132,173,164,224,116,72,14,
    50,72,216,181,155,123,187,53,17,186,70,182,25,126,202,53,31,48,25,112,58,68,89,150,184,219,71,154,80,132,
    154,146,34,156,69,139,121,216,45,99,208,185,252,248,169,82,255,48,141,129,226,175,237,248,212,227,205,198,3,182,
    61,116,182,140,154,188,41,8,65,28,154,102,7,179,251,113,198,118,111,252,219,67,27,130,167,194,130,147,240,52,
    71,7,198,196,138,214,205,232,245,167,30,183,216,89,120,95,148,239,191,116,78,166,229,203,150,25,199,19,238,161,
    201,247,144,254,109,59,126,68,121,186,18,161,182,50,70,55,75,73,210,126,183,204,115,34,80,229,124,204,229,171,
    103,213,175,29,230,38,7,229,95,136,87,154,129,230,29,161,206,135,85,102,242,120,43,208,193,222,201,97,152,137,
    47,78,241,3,64,126,214,202,152,40,140,247,210,201,15,175,81,174,144,146,95,26,104,81,125,135,81,134,19,210,
    25,84,162,58,12,151,189,163,201,167,105,174,119,109,148,155,241,23,223,89,128,72,126,50,43,50,4,178,175,107,
    165,182,160,76,8,11,91,172,85,77,70,109,195,9,106,66,57,171,217,92,170,160,236,169,178,218,109,34,129,120,
    246,201,96,172,176,41,188,65,19,118,164,200,228,62,184,73,155,16,91,181,223,158,176,152,111,50,247,245,169,39,
    5,90,188,181,176,82,193,111,7,138,45,134,53,45,45,53,28,79,146,144,231,248,189,218,186,175,54,207,233,229,
    175,185,138,5,244,12,86,30,30,91,105,124,41,148,79,84,95,222,90,198,224,13,176,182,22,57,246,17,66,46,
    239,209,206,161,225,46,65,109,163,42,40,128,64,106,171,190,78,161,201,10,199,225,152,91,213,132,158,225,213,43,
    104,176,160,90,72,215,60,113,164,64,112,192,74,37,76,29,179,106,144,231,253,193,193,1,23,16,16,60,241,206,
    136,235,23,90,131,198,239,81,94,108,105,14,240,109,12,125,123,98,108,231,40,63,91,195,107,38,72,242,195,239,
    169,214,59,204,248,139,85,238,236,161,200,48,3,66,181,234,98,45,214,68,175,239,231,241,196,191,27,136,58,160,
    9,174,140,226,76,200,188,182,77,36,218,42,35,182,1,113,155,88,91,63,240,156,94,63,6,120,38,43,44,44,
    54,43,39,37,14,121,180,27,181,103,83,7,66,205,7,42,220,45,255,166,195,224,59,22,170,91,3,187,20,16,
    78,239,59,12,1,175,251,99,202,190,23,108,20,6,141,236,241,59,113,255,118,198,107,241,126,4,78,40,174,233,
    45,81,122,124,159,151,166,46,120,197,22,250,216,122,118,225,66,205,137,102,46,121,167,140,167,190,135,100,210,229,
    109,160,140,3,52,238,45,236,50,199,158,147,120,33,78,155,200,246,233,45,184,26,55,150,207,100,253,241,222,227,
    156,95,223,248,66,76,7,94,105,58,247,157,203,74,232,225,162,137,55,39,188,98,129,150,212,172,5,43,179,170,
    42,37,194,33,77,84,36,37,171,231,167,111,214,98,231,28,70,56,35,11,78,128,48,0,220,7,74,246,249,215,
    101,120,6,153,83,10,206,111,122,90,44,117,159,66,236,218,199,245,198,66,180,139,119,164,210,3,99,212,85,10,
    174,211,108,186,94,162,222,158,159,184,88,133,192,104,188,206,91,196,253,51,132,131,38,69,217,156,31,158,143,174,
    86,35,28,57,64,83,166,13,252,243,185,89,0,146,249,74,191,23,199,96,109,106,41,52,119,185,184,243,170,19,
    150,236,243,68,114,238,146,238,170,123,137,222,153,238,227,197,71,89,124,190,233,122,88,253,149,162,36,220,18,99,
    178,240,234,224,192,200,178,247,192,165,209,223,151,90,46,22,170,243,225,3,58,166,210,46,71,6,42,211,192,185,
    204,127,112,35,226,20,74,230,80,228,146,35,235,64,200,204,5,181,201,21,24,15,4,221,176,137,182,185,113,192,
    127,133,83,248,207,72,232,189,104,2,126,227,162,109,210,170,11,25,174,189,118,189,145,44,219,10,223,193,73,7,
    224,240,230,79,221,198,149,27,67,63,12,93,24,138,148,223,157,104,241,234,226,193,253,198,97,35,102,151,62,171,
    220,237,12,232,126,198,7,144,98,87,146,139,178,13,88,147,81,214,46,73,169,71,31,27,144,122,248,68,110,71,
    222,188,10,191,26,120,20,112,248,192,27,69,192,37,133,251,143,208,81,73,216,154,35,81,186,28,43,158,38,126,
    15,233,236,253,109,156,79,239,126,243,96,195,86,73,61,133,99,88,209,65,222,9,250,7,103,230,71,95,19,7,
    171,244,154,139,44,223,133,74,68,233,89,69,224,242,98,24,39,234,169,93,71,213,221,229,121,224,59,123,45,205,
    133,214,67,70,242,147,116,104,206,17,168,15,172,29,62,163,100,12,180,22,75,2,85,87,225,87,236,238,230,60,
    22,194,45,44,241,189,229,85,232,245,250,2,227,31,29,152,222,88,41,108,158,107,39,71,209,153,207,209,62,11,
    74,254,67,167,6,241,165,26,79,249,59,195,57,130,115,227,57,164,201,54,89,217,188,44,87,69,47,63,92,175,
    103,212,192,193,18,54,115,246,170,34,79,145,54,75,120,83,16,196,221,109,152,8,222,52,73,161,12,229,228,58,
    223,203,210,31,121,114,176,169,189,103,44,65,177,32,188,108,34,200,4,79,79,65,49,157,91,135,214,19,208,251,
    254,92,197,9,83,86,241,65,6,46,67,210,72,14,109,57,7,35,122,191,244,28,201,138,199,40,67,125,247,146,
    234,110,255,236,90,52,253,148,64,155,72,56,129,15,46,72,22,57,248,245,193,90,227,11,6,21,118,50,41,11,
    55,16,3,243,104,164,114,117,19,220,62,127,4,185,208,145,240,56,39,47,28,241,121,17,126,205,241,178,229,10,
    125,34,97,15,126,183,52,141,195,104,221,174,79,4,204,202,202,227,184,39,254,29,236,238,71,100,190,213,37,143,
    54,228,82,133,135,135,29,180,84,37,202,15,78,232,253,41,88,191,199,164,254,58,170,127,222,174,214,236,216,179,
    240,95,77,115,187,174,184,234,216,9,128,55,117,142,107,143,8,144,248,182,221,220,48,183,246,64,160,242,200,132,
    236,237,181,177,43,13,249,240,203,230,107,234,86,94,43,154,111,80,4,38,254,63,255,104,243,97,254,38,97,97,
    6,246,200,55,187,200,247,230,136,136,149,91,191,32,64,187,183,89,57,59,214,125,206,202,59,176,206,246,117,96,
    84,94,67,65,124,73,240,49,247,129,223,43,60,195,163,54,57,204,127,127,172,105,41,102,47,7,157,149,248,74,
    91,82,199,223,189,55,138,65,85,179,17,205,212,156,150,170,84,132,140,43,0,221,49,36,108,199,73,52,78,252,
    216,71,38,214,144,106,217,195,0,219,99,122,202,113,34,158,157,102,183,126,34,126,1,112,142,118,107,165,146,82,
    213,8,233,246,207,207,177,211,114,33,15,213,239,166,93,49,241,171,191,116,13,0,23,73,38,22,97,44,33,225,
    186,184,63,213,102,105,45,131,89,226,70,143,31,59,223,232,64,127,125,158,122,35,59,165,102,156,59,97,240,116,
    76,160,107,103,18,108,34,157,120,156,95,48,50,200,220,194,233,71,89,57,30,106,35,42,46,73,102,218,61,245,
    231,101,80,36,234,98,128,163,250,63,75,15,122,221,86,158,53,119,251,241,249,226,20,170,252,175,76,98,96,185,
    221,170,38,51,117,17,230,124,233,94,245,100,199,233,93,193,159,222,47,5,61,217,234,148,86,186,163,187,61,173,
    207,244,84,242,29,57,160,164,150,42,40,189,152,142,11,203,55,137,21,134,183,232,248,16,125,41,12,94,103,42,
    71,44,210,64,187,187,89,46,64,5,38,160,13,9,210,254,250,239,50,180,121,131,203,201,167,42,56,217,206,170,
    184,121,144,37,42,10,63,243,43,6,55,183,190,116,254,159,123,145,133,87,76,119,139,205,227,50,202,200,124,210,
    70,115,219,124,24,39,66,154,200,100,27,52,159,28,144,123,176,166,11,107,34,226,89,39,192,193,50,176,18,92,
    86,232,3,104,127,189,91,121,105,239,114,13,201,204,210,12,140,170,4,40,190,51,188,20,0,32,6,204,18,160,
    181,112,235,216,199,49,124,242,99,167,27,47,93,169,62,118,156,242,127,7,156,73,201,112,189,198,25,78,103,45,
    165,252,156,208,195,47,253,242,98,168,243,189,247,116,154,217,157,125,237,206,10,157,21,122,122,175,207,17,191,214,
    171,166,85,100,113,80,196,164,220,220,228,88,61,38,173,150,170,10,13,57,79,238,238,14,45,12,107,183,10,5,
    163,146,95,103,105,43,64,239,61,40,144,249,243,216,122,135,67,70,189,90,167,29,236,171,14,155,73,151,168,14,
    18,21,178,205,121,134,162,166,251,25,63,123,40,4,93,82,162,144,161,89,149,246,235,103,228,241,111,130,123,53,
    128,248,206,139,98,97,116,102,93,46,193,188,205,217,181,71,115,224,206,130,52,65,49,232,201,62,54,126,34,193,
    254,128,63,139,26,210,67,110,114,73,21,18,160,236,182,253,18,118,63,244,102,237,36,163,251,85,194,143,72,113,
    240,107,195,218,204,129,75,215,251,78,39,88,54,168,7,127,67,225,117,119,149,71,7,157,252,200,249,169,4,173,
    221,91,6,40,226,32,141,71,161,39,112,149,2,162,86,117,129,10,183,71,131,176,249,215,234,27,63,246,55,119,
    201,226,132,203,219,241,175,151,209,127,90,123,51,194,218,15,46,163,240,174,225,61,100,126,160,142,227,27,202,71,
    216,184,41,2,253,251,248,113,53,146,19,81,141,12,233,216,160,45,226,114,79,157,126,56,216,54,197,188,234,246,
    116,222,232,175,186,115,135,225,164,174,89,25,25,169,147,136,134,144,94,194,23,205,148,12,142,73,72,24,150,114,
    95,109,4,147,233,58,74,110,241,210,234,216,154,20,46,213,149,165,26,115,241,192,36,158,53,47,107,201,65,195,
    6,8,29,99,161,152,215,19,90,61,100,80,241,137,170,98,215,40,183,75,215,235,160,14,100,94,236,178,13,58,
    175,62,104,26,4,238,139,6,205,190,249,30,137,95,227,146,74,162,151,35,179,79,240,238,107,132,180,190,200,201,
    21,116,202,139,43,30,162,56,23,99,174,94,75,171,200,8,115,231,1,200,49,93,211,160,233,137,147,38,138,143,
    67,222,17,139,146,65,97,68,12,251,36,148,146,179,91,111,63,71,227,67,80,31,50,32,48,149,63,239,196,123,
    70,141,146,105,84,80,179,198,72,84,191,232,8,73,46,171,58,77,90,117,182,33,169,239,97,33,9,124,220,54,
    123,231,61,15,172,26,34,238,226,185,219,131,9,104,140,126,156,165,254,185,79,33,66,236,134,77,217,67,21,66,
    67,205,173,41,7,76,68,207,213,87,209,177,47,231,127,28,246,99,164,188,169,60,89,68,138,68,76,46,81,130,
    137,144,47,0,144,34,107,209,66,85,180,132,178,205,67,204,92,246,51,40,144,31,95,23,173,116,228,0,165,55,
    128,248,248,208,130,6,123,187,158,156,193,251,254,43,204,238,241,99,88,160,250,6,133,121,76,183,60,128,139,223,
    246,32,122,239,149,80,235,39,8,253,248,201,235,10,230,246,176,130,185,83,27,7,45,154,38,55,33,138,160,130,
    80,186,48,44,147,176,194,192,58,211,126,132,128,40,224,127,186,81,251,144,106,97,200,117,101,207,88,224,174,118,
    238,73,137,154,15,107,233,209,186,98,125,163,164,10,235,64,108,58,56,91,131,216,233,197,191,34,213,71,174,112,
    141,155,244,169,24,245,30,14,55,220,242,82,81,70,237,54,161,33,126,208,146,38,94,242,253,133,70,133,160,167,
    107,154,5,98,192,151,167,10,243,220,20,135,123,204,201,238,2,86,243,169,182,182,102,161,167,61,187,82,233,64,
    237,115,63,227,26,103,171,202,83,81,123,221,250,92,166,18,130,91,98,82,173,218,254,244,143,188,185,55,150,120,
    75,1,58,127,166,243,244,135,133,119,34,33,56,113,207,131,126,12,150,176,41,79,129,236,87,151,71,49,239,74,
    191,253,236,83,124,124,110,99,145,195,221,25,187,130,118,64,129,33,134,216,166,113,227,196,119,215,98,175,236,27,
    59,2,195,43,38,82,40,64,205,253,208,19,174,161,236,148,65,197,201,34,167,5,48,28,5,154,190,116,62,211,
    99,187,126,216,39,222,131,148,170,102,117,214,44,126,20,173,8,229,15,120,184,236,101,81,251,78,255,247,252,112,
    249,99,30,86,159,244,126,71,247,63,202,50,192,198,40,188,165,223,78,40,112,193,252,77,96,140,208,107,177,75,
    109,193,90,89,81,12,172,1,183,185,30,179,79,60,140,158,227,156,163,152,83,60,198,88,155,108,111,255,0,249,
    86,134,183,156,4,178,172,104,114,84,99,75,121,43,158,160,90,101,80,179,176,194,85,138,180,5,144,217,251,141,
    65,252,125,246,169,55,233,224,73,22,214,54,23,215,50,169,231,254,194,55,211,217,209,174,91,1,195,158,198,88,
    0,128,70,239,44,109,73,124,199,93,86,183,194,110,109,139,235,148,172,22,53,77,163,119,218,143,186,19,79,239,
    145,133,179,179,104,219,115,49,204,188,229,137,253,93,113,119,215,221,53,161,201,99,211,179,163,82,117,196,61,80,
    91,199,158,168,190,225,118,166,78,160,15,86,178,140,42,138,23,86,31,181,182,207,33,128,9,163,12,14,217,89,
    102,14,131,127,162,206,71,113,148,119,203,123,124,89,2,148,247,216,62,155,252,116,207,79,6,197,73,119,106,117,
    106,143,232,107,129,163,149,162,148,9,199,249,205,144,188,192,58,79,177,229,212,96,14,119,161,79,14,56,218,180,
    113,104,71,46,97,92,4,135,194,36,73,90,10,144,189,50,152,84,255,48,236,242,132,157,206,20,10,13,18,118,
    45,198,60,146,224,26,202,1,30,23,190,82,124,162,194,7,190,110,109,72,31,124,114,90,187,200,116,174,193,155,
    114,193,117,243,60,15,152,215,209,108,87,243,215,59,189,204,209,172,127,143,155,86,8,154,45,72,120,18,153,172,
    3,79,163,44,236,69,148,39,109,74,66,204,118,203,240,212,103,240,184,27,135,219,92,104,242,228,163,235,93,38,
    107,205,212,252,136,246,25,143,247,119,143,14,133,46,234,205,253,190,31,16,124,56,226,246,250,121,38,245,207,111,
    101,15,85,24,81,146,244,12,78,157,62,89,168,7,43,88,102,67,244,242,184,124,134,210,54,144,183,86,35,8,
    103,64,53,7,20,7,163,104,219,203,75,175,138,131,214,51,174,123,165,111,202,189,31,56,227,126,99,97,80,234,
    62,73,119,40,178,234,86,176,50,173,242,127,134,157,37,68,84,133,225,155,112,186,177,152,17,2,171,143,200,8,
    9,86,41,65,85,227,46,198,101,104,192,219,191,24,4,21,188,24,183,83,56,92,62,49,152,163,194,101,24,112,
    118,70,182,125,115,76,61,241,92,111,61,177,233,151,151,158,151,225,28,46,217,250,5,114,57,96,78,43,144,8,
    120,232,168,64,147,86,178,14,11,98,158,148,116,48,40,157,129,229,98,30,43,243,130,22,167,179,148,76,252,88,
    44,227,36,108,74,220,20,14,104,146,138,87,118,98,108,9,109,132,222,207,102,189,143,178,78,119,118,26,157,159,
    85,219,13,153,83,177,202,53,103,187,85,106,40,238,186,163,84,37,178,124,37,144,115,66,187,223,56,171,1,25,
    250,44,125,99,223,153,91,25,162,231,101,102,182,172,178,5,13,24,178,2,13,190,140,103,187,35,225,6,229,219,
    178,130,154,234,163,65,42,75,219,241,248,250,104,20,132,10,32,128,4,146,239,232,143,39,99,19,134,237,161,242,
    198,73,160,165,172,128,55,124,185,22,237,152,105,147,101,253,241,114,248,81,27,114,172,112,149,103,104,98,103,83,
    101,64,253,63,42,235,78,240,164,207,194,204,135,76,8,212,128,146,54,150,60,23,248,8,122,13,136,136,71,114,
    70,120,81,124,233,215,227,67,50,219,128,7,31,212,109,221,38,17,131,70,95,22,246,8,252,58,220,189,61,250,
    132,101,19,247,82,215,101,41,208,221,88,50,106,208,228,5,105,240,95,231,118,89,45,19,153,107,251,230,62,112,
    196,121,236,174,13,215,110,19,70,177,95,207,58,83,117,96,216,144,53,246,233,73,179,83,156,0,140,82,255,193,
    153,185,81,250,243,249,25,156,192,116,98,48,241,78,145,206,241,60,203,118,211,95,33,228,0,12,126,206,115,53,
    151,125,241,58,101,254,232,164,87,172,96,221,190,109,194,68,224,39,13,174,21,20,190,203,96,69,56,28,112,137,
    55,247,105,50,20,3,69,170,191,48,186,247,251,36,175,18,1,221,41,14,151,155,150,54,152,188,241,223,66,167,
    163,150,157,195,222,11,165,121,35,18,251,95,73,221,237,253,204,190,55,91,30,156,209,112,105,28,28,53,55,180,
    224,178,28,171,158,25,103,221,248,139,3,38,149,32,67,155,241,223,7,35,247,25,159,17,50,86,82,158,103,78,
    76,192,132,11,44,33,224,199,68,150,186,167,28,176,120,49,81,72,42,236,215,123,44,1,128,2,209,248,249,249,
    137,41,74,118,147,214,35,55,149,137,211,19,159,175,202,160,133,53,122,123,7,254,127,60,22,75,72,148,236,219,
    216,169,117,24,18,209,253,17,215,177,102,200,82,135,144,136,255,16,3,157,147,237,52,241,196,88,190,37,91,110,
    57,218,199,231,90,108,106,254,163,225,11,13,70,169,249,96,120,149,150,223,63,57,193,113,129,212,187,107,155,184,
    82,1,253,162,235,10,103,219,172,174,151,174,196,135,50,78,13,102,93,37,2,39,215,157,93,170,153,212,16,69,
    86,135,254,97,158,159,38,239,8,14,66,3,142,50,193,102,160,47,25,70,118,123,68,194,156,133,13,135,173,220,
    62,234,210,247,145,14,117,190,170,210,156,5,61,235,1,212,59,233,97,255,185,12,231,97,121,207,82,26,61,104,
    216,144,198,37,15,134,3,224,143,193,126,28,16,119,129,26,52,241,172,4,161,103,229,39,90,236,99,176,14,188,
    69,169,193,134,216,164,165,95,77,157,223,189,251,210,245,144,192,208,101,230,156,181,135,2,36,239,22,30,238,61,
    59,18,65,102,79,202,85,164,28,47,122,214,134,18,107,139,59,224,110,42,218,88,112,198,57,232,217,220,198,2,
    244,192,146,195,29,59,153,87,115,56,128,159,61,134,161,16,247,168,53,161,240,132,179,165,17,49,226,24,188,143,
    159,60,30,223,81,196,208,9,225,160,42,8,161,242,104,10,33,96,173,157,164,150,50,66,202,209,214,105,189,244,
    254,145,134,205,76,235,16,246,181,160,90,135,189,181,214,189,250,120,31,161,202,34,68,45,186,135,177,30,170,112,
    78,189,9,131,55,150,226,41,244,2,223,118,14,163,134,226,89,250,57,84,227,84,158,185,131,1,17,248,38,93,
    17,64,191,157,125,170,131,144,245,154,181,124,245,75,191,109,230,61,203,35,223,237,41,144,227,121,237,126,215,119,
    57,121,21,233,134,84,1,152,254,99,199,127,223,163,172,165,131,87,76,64,79,68,77,81,189,173,32,115,158,122,
    230,143,140,162,155,94,237,234,174,86,59,82,47,47,249,235,2,177,15,52,120,114,106,128,209,253,239,13,69,255,
    225,52,146,111,208,54,192,81,222,98,152,95,249,17,14,66,1,167,96,250,106,245,40,174,181,170,157,41,218,111,
    31,81,14,68,132,86,167,70,23,93,79,247,248,68,190,181,238,39,217,168,8,85,189,113,232,54,151,125,171,118,
    188,94,150,20,182,75,8,148,1,217,21,66,32,152,56,204,31,94,115,78,51,193,249,244,39,236,239,43,82,110,
    203,17,252,187,254,72,40,191,7,35,78,3,54,190,222,109,60,57,164,169,1,165,20,248,63,231,7,154,203,202,
    247,66,101,227,140,144,221,70,195,252,103,156,76,8,96,218,225,193,121,128,143,253,78,248,249,78,252,174,63,110,
    202,184,192,188,251,199,178,194,235,114,85,121,50,212,70,0,184,173,7,188,96,116,51,156,72,15,57,104,163,167,
    76,14,35,228,111,194,55,135,250,237,229,128,242,18,49,34,208,187,167,40,47,56,99,127,97,231,155,145,243,43,
    63,215,138,158,217,171,244,254,164,64,249,131,94,90,82,104,118,236,239,80,165,17,30,13,249,222,159,119,31,26,
    146,4,101,22,97,9,44,59,191,162,47,87,121,60,125,126,156,203,149,127,189,155,124,133,219,91,168,52,71,176,
    84,46,140,72,201,214,47,186,208,184,90,255,100,23,181,142,4,2,198,96,128,119,150,219,118,158,3,236,118,209,
    251,213,108,43,194,19,27,101,234,169,15,181,30,125,66,114,70,232,243,5,92,110,213,235,93,104,11,157,151,59,
    60,46,222,63,33,214,235,226,127,219,61,207,195,210,237,114,159,231,228,223,121,59,34,57,121,37,109,123,97,136,
    90,207,196,37,160,160,54,103,218,228,25,135,243,130,64,157,151,82,132,56,24,18,150,40,47,229,158,173,129,203,
    203,139,249,181,247,58,254,217,198,10,125,245,217,195,14,30,49,172,211,226,31,113,200,129,74,75,30,88,246,97,
    99,208,156,142,191,47,51,34,30,134,140,239,38,89,150,161,219,53,96,14,235,250,199,154,4,57,200,217,33,178,
    248,94,94,199,1,90,161,244,115,162,112,229,55,129,131,124,199,159,92,98,82,77,221,101,247,206,171,201,65,156,
    47,123,44,28,214,112,120,66,214,68,107,249,37,33,32,79,196,110,109,253,240,247,94,94,85,106,86,47,187,167,
    246,207,169,167,246,196,13,29,201,139,153,170,155,213,84,210,205,127,205,230,17,255,85,49,48,92,195,41,18,183,
    195,20,141,39,0,147,124,66,77,69,254,61,65,212,207,5,175,91,49,128,216,49,87,109,27,251,237,81,150,163,
    58,94,134,121,211,225,247,155,47,153,74,154,19,129,15,235,246,90,47,17,145,76,229,31,101,126,223,29,31,41,
    35,40,114,216,140,82,6,184,13,140,138,127,43,63,0,0,1,185,209,28,83,120,90,78,87,36,119,36,247,129,
    230,145,137,147,252,9,16,81,232,89,5,162,246,246,183,188,34,0,212,149,137,100,104,246,143,163,234,247,169,159,
    199,0,34,13,45,136,73,174,220,152,214,208,205,171,123,121,181,65,151,77,226,76,186,22,131,137,65,61,209,61,
    26,71,143,207,119,41,199,18,189,254,50,242,63,112,163,159,140,134,77,57,162,166,7,229,160,68,154,54,53,198,
    244,215,82,216,35,10,216,137,83,136,111,129,166,213,174,234,23,164,135,5,126,83,135,50,167,143,20,232,173,13,
    146,146,106,189,206,223,64,236,102,165,67,20,49,108,210,46,216,100,71,0,158,93,52,164,38,11,25,192,163,63,
    82,6,185,232,99,197,206,126,197,228,167,28,13,44,100,23,255,168,64,3,125,253,235,145,243,191,212,192,123,89,
    216,206,208,148,222,248,181,146,127,31,22,59,115,253,166,215,51,92,110,166,83,195,169,10,124,236,104,22,246,28,
    234,237,254,210,95,31,111,249,62,88,238,150,160,51,98,76,97,78,119,151,242,146,31,67,94,170,75,107,224,185,
    252,77,209,23,226,45,166,24,2,197,23,153,253,103,252,204,49,110,185,204,124,25,146,19,104,126,134,7,11,121,
    64,132,134,208,243,141,195,124,101,46,240,3,182,151,175,124,144,144,225,72,171,110,186,72,26,140,152,176,42,171,
    40,230,121,26,124,26,123,54,224,241,145,51,97,63,158,163,123,79,69,190,34,129,239,63,239,185,84,159,73,144,
    117,190,63,106,116,158,32,160,56,182,136,49,230,228,90,49,82,220,74,184,167,110,125,46,81,188,20,114,136,59,
    115,209,203,143,105,200,199,64,107,109,233,60,236,189,69,27,216,151,232,190,255,178,76,153,249,237,97,89,111,197,
    75,150,99,54,127,132,191,27,184,106,90,115,211,122,80,188,207,194,228,152,241,77,35,95,230,255,3,43,102,15,
    154,36,30,0,0
};
