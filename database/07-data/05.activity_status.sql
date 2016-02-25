INSERT INTO activity_status(id, name, description)
VALUES (
1,
'Submitted',
'The status of a freshly added activity.');

INSERT INTO activity_status(id, name, description)
VALUES (
2,
'PendingStart',
'The status of an activity that has been added to the resource manager''s execution queue.');

INSERT INTO activity_status(id, name, description)
VALUES (
3,
'NeedsInput',
'The activity has been suspended due to lack of necessary input.');

INSERT INTO activity_status(id, name, description)
VALUES (
4,
'Running',
'The activity is in progress.');

INSERT INTO activity_status(id, name, description)
VALUES (
5,
'Paused',
'The activity has been paused by the user.');

INSERT INTO activity_status(id, name, description)
VALUES (
6,
'Finished',
'The activity has been completed.');

INSERT INTO activity_status(id, name, description)
VALUES (
7,
'Cancelled',
'The activity has been cancelled by the user.');

INSERT INTO activity_status(id, name, description)
VALUES (
8,
'Error',
'The activity has encountered an error and has stopped.');
