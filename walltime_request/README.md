# HPC Walltime Request

Code for computing the optimal request time for a stochastic application when submitted on an HPC scheduler, based on historic information about previous runs.
Any code wanting to use this code has to include two classes:
- `import WorkloadCDF`
- `import OptimalSequence`

### 1. Prepare the data

Create a list with historic resource usage information. If walltime is the resource under study, `data` is a list of walltimes for past runs. Create a WorkloadCDF object on the historic data and compute the discrete CDF based on the data.

```python
wf = WorkloadCDF.WorkloadCDF(data)
optimal_data, optimal_cdf = wf.compute_discrete_cdf()
```

### 2. Compute the sequence of requests

Create a Sequence object based on the discrete data and CDF and compute the request sequences.

```python
handler = OptimalSequence.TODiscretSequence(max(data), optimal_data, optimal_cdf)
optimal_sequence = handler.compute_request_sequence()
```


### 3. Compute the cost of a sequence of requests

Compute the cost of a given sequence by creating a Cost object based on the sequence and runing it on the new data. The cost represents the average time used by each submission for all reservations. This time represents all the failed reservation together with the sucessful one. For example, for two submissions one of 10 and another of 15 hours, the cost of the sequence [8, 11, 16] is the average between `8 + 10` (the first submission will fail when requesting 8hs and will succeed the second time) and `8 + 11 + 15`.

```python
cost_handler = WorkloadCDF.LogDataCost(optimal_sequence)
optimal_cost = cost_handler.compute_cost(new_data)
```
