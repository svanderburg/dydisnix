{
  testtarget1 = {
    hostname = "testtarget1";
    supportedTypes = [ "echo" "process" "wrapper" ];
    mem = 524288;
    zone = "US";
  };
  
  testtarget2 = {
    hostname = "testtarget2";
    supportedTypes = [ "echo" "process" "wrapper" ];
    mem = 655360;
    zone = "Europe";
  };
}
