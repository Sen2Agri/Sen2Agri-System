INSERT INTO activity_status(name, description)
VALUES (
'Submitted',
'The status of a freshly added activity.');

INSERT INTO activity_status(name, description)
VALUES (
'PendingStart',
'The status of an activity that has been added to the resource manager''s execution queue.');

INSERT INTO activity_status(name, description)
VALUES (
'NeedsInput',
'The activity has been suspended due to lack of necessary input.');

INSERT INTO activity_status(name, description)
VALUES (
'Running',
'The activity is in progress.');

INSERT INTO activity_status(name, description)
VALUES (
'Paused',
'The activity has been paused by the user.');

INSERT INTO activity_status(name, description)
VALUES (
'Finished',
'The activity has been completed.');

INSERT INTO activity_status(name, description)
VALUES (
'Cancelled',
'The activity has been cancelled by the user.');

INSERT INTO activity_status(name, description)
VALUES (
'Error',
'The activity has encountered an error and has stopped.');
