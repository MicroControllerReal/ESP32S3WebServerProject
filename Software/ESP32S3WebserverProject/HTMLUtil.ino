String repl[][2] = {
  {"\t","&#09;"},                        //&#x09;  Horizontal Tab
  {"\n","<br>"},                         //&#x10;  Line Feed
                                         //    &#32; &#x20;  Space
                                         //! ! &#33; &#x21;  Exclamation Point
  {"\"","&quot;"},                       //;  &#34; &#x22;  Double Quote
                                         //# # &#35; &#x23;  Number Sign
  {"&","&amp;"},                         // &#38; &#x26;  Ampersand
                                         //' ' &#39; &#x27;  Single Quote
                                         //( ( &#40; &#x28;  Left Parenthesis
                                         //) ) &#41; &#x29;  Right Parenthesis
                                         //* * &#42; &#x2A;  Asterisk (Star)
                                         //, , &#44; &#x2C;  Comma
                                         //- - &#45; &#x2D;  Hyphen
                                         //. . &#46; &#x2E;  Period
                                         /// / &#47; &#x2F;  Forward Slash
                                         //: : &#58; &#x3A;  Colon
                                         //; ; &#59; &#x3B;  Semi-Colon
                                         //? ? &#63; &#x3F;  Question Mark
                                         //@ @ &#64; &#x40;  At Sign
                                         //[ [ &#91; &#x5B;  Left Square Bracket
                                         //\ \ &#92; &#x5C;  Back Slash
                                         //] ] &#93; &#x5D;  Right Square Bracket
                                         //^ ^ &#94; &#x5E;  Caret
                                         //_ _ &#95; &#x5F;  Underscore
                                         //{ { &#123;  &#x7B;  Left Curly Brace
                                         //| | &#124;  &#x7C;  Vertical Bar
                                         //} } &#125;  &#x7D;  Right Curly Brace
  {"~","&tilde;"},                       // &#126;  &#x7E;  Vertical Bar
  {"‚","&sbquo;"},                       // &#130;  &#x82;  Single Low Quote
  {"„","&dbquo;"},                       // &#132;  &#x84;  Double Low Quote
  {"…","&#133;"},                       // &#x85;  Elipsis
  {"†","&dagger;"},                      // &#134;  &#x86;  Dagger
  {"‡","&Dagger;"},                      // &#135;  &#x87;  Double Dagger
  {"‹","&lsaquo;"},                      // &#139;  &#x8B;  Left Single Angle Quote
  {"‘","&lsquo;"},                       // &#145;  &#x91;  Left Single Quote
  {"’","&rsquo;"},                       // &#146;  &#x92;  Right Single Quote
  {"“","&ldquo;"},                       // &#147;  &#x93;  Left Double Quote
  {"”","&rdquo;"},                       // &#148;  &#x94;  Right Double Quote
  {"•","&#149;"},                        // &#x95;  Small Bullet
  {"–","&ndash;"},                       // &#150;  &#x96;  En Dash
  {"—","&mdash;"},                      // &#151;  &#x97;  Em Dash
  {"™","&trade;"},                       // &#153;  &#x99;  Trademark
  {"›","&rsaquo;"},                       // &#155;  &#x9B;  Right Single Angle Quote
  {"\u00A0","&nbsp;"},                    // &#160;  &#xA0;  Non-Breaking Space
  {"¡","&iexcl;"},                        // &#161;  &#xA1;  Inverted Exclamation Point
  {"¦","&brvbar;"},                       // &#166;  &#xA6;  Broken Vertical Bar
  {"©","&copy;"},                         // &#169;  &#xA9;  Copyright
  {"ª","&ordf;"},                         // &#170;  &#xAA;  Feminine Ordinal Indicator
  {"«","&laquo;"},                        // &#171;  &#xAB;  Left Angle Quote
  {"¬","&not;"},                          // &#172;  &#xAC;  Not Sign
  {"­","&shy;"},                          // &#173;  &#xAD;  Soft Hyphen
  {"®","&reg;"},                          // &#174;  &#xAE;  Registered Symbol
  {"°","&deg;"},                          // &#176;  &#xB0;  Degree
  {"²","&sup2;"},                         // &#178;  &#xB2;  Superscript 2
  {"³","&sup3;"},                         // &#179;  &#xB3;  Superscript 3
  {"µ","&micro;"},                        // &#181;  &#xB5;  Micro Sign
  {"¶","&para;"},                         // &#182;  &#xB6;  Pilcrow (Paragraph Sign)
  {"·","&middot;"},                       // &#183;  &#xB7;  Middle Dot
  {"¹","&sup1;"},                         // &#185;  &#xB9;  Superscript 1
  {"º","&ordm;"},                         // &#186;  &#xBA;  Masculine Ordinal Indicator
  {"»","&raquo;"},                        // &#187;  &#xBB;  Right Angle Quote
  {"¿","&iquest;"},                       // &#191;  &#xBF;  Inverted Question Mark
  {"Ä","&Auml;"},                         // &#196; A Umlaut
  {"ä","&auml;"},                         // &#228; a Umlaut
  {"Ö","&Ouml;"},                         // &#214; O Umlaut
  {"ö","&ouml;"},                         // &#246; o Umlaut
  {"Ü","&Uuml;"},                         // &#220; U Umlaut
  {"ü","&uuml;"},                         // &#252; u Umlaut
  {"ß","&szlig;"},                        // &#223; Eszett
  {"Ä","&Auml;"},                         // &#196; A Umlaut
  {"ä","&auml;"},                         // &#228; a Umlaut
  {"Ë","&Euml;"},                         // &#203; E Umlaut
  {"ë","&euml;"},                         // &#235; e Umlaut
  {"Ï","&Iuml;"},                         // &#207; I Umlaut
  {"ï","&iuml;"},                         // &#239; i Umlaut
  {"À","&Agrave;"},                       // &#192; A mit Accent Grave (Gravis)
  {"à","&agrave;"},                       // &#224; a mit Accent Grave (Gravis)
  {"Á","&Aacute;"},                       // &#193; A mit Accent Aigu (Akut)
  {"á","&aacute;"},                       // &#225; a mit Accent Aigu (Akut)
  {"Â","&Acirc;"},                        // &#194; A mit Zirkumflex
  {"â","&acirc;"},                        // &#226; a mit Zirkumflex
  {"Ç","&Ccedil;"},                       // &#199; C mit Cedille
  {"ç","&ccedil;"},                       // &#231; c mit Cedille
  {"È","&Egrave;"},                       // &#200; E mit Accent Grave (Gravis)
  {"è","&egrave;"},                       // &#232; e mit Accent Grave (Gravis)
  {"É","&Eacute;"},                       // &#201; E mit Accent Aigu (Akut)
  {"é","&eacute;"},                       // &#233; e mit Accent Aigu (Akut)
  {"Ê","&Ecirc;"},                        // &#202; E mit Zirkumflex
  {"ê","&ecirc;"},                        // &#234; e mit Zirkumflex
  {"Ñ","&Ntilde;"},                       // &#209; N mit Tilde
  {"ñ","&ntilde;"},                       // &#241; n mit Tilde
  {"Ò","&Ograve;"},                       // &#210; O mit accent grave (Gravis)
  {"ò","&ograve;"},                       // &#242; o mit accent grave (Gravis)
  {"Ó","&Oacute;"},                       // &#211; O mit accent aigu (Akut)
  {"ó","&oacute;"},                       // &#243; o mit accent aigu (Akut)
  {"Ô","&Ocirc;"},                        // &#212; O mit Zirkumflex
  {"ô","&ocirc;"},                        // &#244; o mit Zirkumflex
  {"õ","&otilde;"},                       // &#245; o mit Tilde
  {"Ÿ","&Yuml;"},                         // &#195; Y Umlaut
  {"ÿ","&yuml;"},                         // &#255; y Umlaut
  {"℅","&#8453;"},                        // &#x2105;  Care Of
  {"ⁿ","&#8319;"},                         // &#x207F;  Superscript N
  {"§","&sect;"},                          // &#167;  &#xA7;  Section Mark
  {"―","&#8213;"},                        // &#x2015;  Horizontal Bar
  {"‣","&#8227;"},                         // &#x2023;  Triangle Bullet
  {"‼","&#8252;"},                         // &#x203C;  Double Exclamation Point
  {"№","&#8470;"},                        // &#x2116;  Number Word
  {"€","&#8364;"},
  {"£","&#0163;"},
  {"¥","&#165;"}
};

void textToHTML(char txt[], int len){
  String s = txt;                         // Make a copy of the original text
  for(auto r : repl){                     // Loop through all known HTML items
    s.replace(r[0],r[1]);                 // Replace them with their coded version
  }
  strcpy(txt,"<meta charset=\"UTF-8\">"); // Initiate the HTML text to UTF-8
  char tmp[len];                          // char buffer to hold String
  s.toCharArray(tmp, len-22);             // Copy String to char buffer
  strcat(txt,tmp);                        // Append String to HTML text to return to caller
}
