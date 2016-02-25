INSERT INTO event_type(
id,
name, 
description)
VALUES (
1,
'TaskRunnable',
'Event triggered when the processing of a module can be started.');

INSERT INTO event_type(
id,
name, 
description)
VALUES (
2,
'TaskFinished',
'Event triggered when the processing of a module is completed.');

INSERT INTO event_type(
id,
name, 
description)
VALUES (
3,
'ProductAvailable',
'Event triggered when a new product becomes available.');

INSERT INTO event_type(
id,
name, 
description)
VALUES (
4,
'JobCancelled',
'Event triggered when a job has been cancelled by the user.');

INSERT INTO event_type(
id,
name, 
description)
VALUES (
5,
'JobPaused',
'Event triggered when a job has been paused by the user.');

INSERT INTO event_type(
id,
name, 
description)
VALUES (
6,
'JobResumed',
'Event triggered when a job has been resumed by the user.');

INSERT INTO event_type(
id,
name, 
description)
VALUES (
7,
'JobSubmitted',
'Event triggered when a job request has been submitted.');

INSERT INTO event_type(
id,
name, 
description)
VALUES (
8,
'StepFailed',
'Event triggered when the execution of a step has encountered an error.');


