{stdenv}:

assert "foo" == "bar";

stdenv.mkDerivation {
  name = "testService1B";
  
  buildCommand = ''
    echo "testService1B" > $out
  '';
}
