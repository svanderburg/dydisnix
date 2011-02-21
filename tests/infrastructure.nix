{
  testtarget1 = {
    hostname = "testtarget1";
    supportedTypes = [ "echo" "process" "wrapper" ];
    mem = 524288;
    zone = "US";
    cost = 100;
    access = "public";
  };
  
  testtarget2 = {
    hostname = "testtarget2";
    supportedTypes = [ "echo" "process" "wrapper" ];
    mem = 655360;
    zone = "Europe";
    cost = 200;
    access = "private";
  };
}
