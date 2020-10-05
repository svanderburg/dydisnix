{
  test1 = {
    properties = {
      hostname = "test1";
      description = "First test machine";
    };

    # No containers
  };

  test2 = {
    properties = {
      hostname = "test2";
      description = "Second test machine";
    };

    containers = {
      process = {};
      wrapper = {};
      mysql-database = {
        mysqlPort = 3306;
      };
      tomcat-webapplication = {};
    };
  };
}
