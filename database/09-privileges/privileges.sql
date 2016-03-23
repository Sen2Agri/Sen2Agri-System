-- Privileges: sen2agri
GRANT ALL PRIVILEGES ON DATABASE sen2agri TO admin;

-- Privileges: sen2agri
GRANT ALL PRIVILEGES ON DATABASE sen2agri TO "sen2agri-service";

GRANT SELECT, INSERT, UPDATE, DELETE
ON ALL TABLES IN SCHEMA public 
TO admin;

GRANT SELECT, INSERT, UPDATE, DELETE
ON ALL TABLES IN SCHEMA public 
TO "sen2agri-service";

