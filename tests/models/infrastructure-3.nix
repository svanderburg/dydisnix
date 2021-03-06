{
  testtarget1 = {
    properties = {
      hostname = "testtarget1";
      supportedTypes = [ "echo" "process" "wrapper" ];
      mem = 524288;
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
      mem = 655360;
      zone = "Europe";
      cost = 200;
      access = "private";
      priority = 1;
    };
  };

  testtarget3 = {
    properties = {
      hostname = "testtarget3";
      supportedTypes = [ "echo" "process" "wrapper" ];
      mem = 655360;
      zone = "Europe";
      cost = 200;
      access = "private";
      priority = 1;
    };
  };
}
