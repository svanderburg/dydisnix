{pkgs, system, distribution, invDistribution}:

rec {
  errordb = {
    name = "errordb";
    type = "postgresql-database";
    description = "Error database backend";
    group = "services";
  };

  loggingdb = {
    name = "loggingdb";
    type = "postgresql-database";
    description = "Logging database backend";
    group = "services/logging";
  };

  loggingapi = {
    name = "loggingapi";
    dependsOn = {
      inherit loggingdb errordb;
    };
    type = "tomcat-webapplication";
    description = "Logging API";
    group = "services/logging";
  };

  reportingdb = {
    name = "reportingdb";
    type = "postgresql-database";
    description = "Reporting database backend";
    group = "services/reporting";
  };

  reportingapi = {
    name = "reportingapi";
    dependsOn = {
      inherit reportingdb errordb;
    };
    type = "tomcat-webapplication";
    description = "Reporting API";
    group = "services/reporting";
  };

  portaldb = {
    name = "portaldb";
    type = "tomcat-webapplication";
    description = "Database backend for the Portal application";
  };

  portal = {
    name = "portal";
    type = "tomcat-webapplication";
    dependsOn = {
      inherit loggingapi reportingapi portaldb;
    };
    description = "Portal web application";
  };
}
