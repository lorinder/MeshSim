Description of a results.db

params
------
This one describes parameter sets that were simulated.

id, dir, and the parameters from enum.json


trace_app_rx
------------
One row for each trace-app-rx-*.txt file found.

id		unique ID
params_id	the parameter set this belongs to
id_in_params	(unique enumeration within parameter set; probably not
		useful)
filename	the name of the text file


trace_app_rx_samples
--------------------
One row for each row in a trace-app-rx-*.txt file.

trace_app_rx_id	the trace_app_rx.id for this row.
time		timestamp of the row
bytes_recv	cumulative bytes received by this time


trace_app_rx_summaries
----------------------
Summary of one trace-app-rx-*.txt data

trace_app_rx_id	the file this belongs to.
max_bytes_recv
min_bytes_recv
max_time
min_time
