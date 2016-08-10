{
  testtarget1 = {
    properties = {
      hostname = "testtarget1";
      supportedTypes = [ "echo" "process" "wrapper" ];
      mem = 1; # Rediculously low
      zone = "US";
      cost = 100;
      access = "public";
      priority = 2;
    };
  };
  
  testtarget2 = {
    properties = {
      hostname = "testtarget2";
      supportedTypes = [ "echo" "process" "wrapper" ];
      mem = 1; # Rediculously low
      zone = "Europe";
      cost = 200;
      access = "private";
      priority = 1;
    };
  };
}
