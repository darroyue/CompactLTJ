# Ring

Repository for the source code of the engine presented at the paper Worst-case Optimal Graph Joins in Almost No Space.

## Instructions

To run our code, **we have to install an extended version of the library SDSL**. Go to [this repository](https://github.com/wangTheTiger/sdsl-lite) and follow the instructions.

After the extended version of SDSL is installed, we have to clone this repository and follow these steps:

1. : Build the project: Compiles everything into our **build** folder:
    - `cmake -Bbuild/ -DCMAKE_BUILD_TYPE=Release && cmake --build build/`

Alternatively older CMAKE versions require the following different set of commands:
-```Bash
-mkdir build
-cd build
-cmake ..
-make
-```

Check that we do not have any errors.

2. Download the version of Wikidata that we want to use:

- [Wikidata Filtered (about 80M triples)](http://compact-leapfrog.tk/files/wikidata-filtered-enumerated.dat).
- [Wikidata (about 1000M triples)](http://compact-leapfrog.tk/files/wikidata-enumerated.dat.gz). Note that we have to decompress this file.

Now put the .dat file inside a folder.

3. Then, we have to create the index. After compiling the code we should have an executable called `build-index` in `build`. Now run:

```Bash
./build-index <absolute-path-to-the-.dat-file> <type-ring>
```

`<type-ring>` can take two values: ring or c-ring.
This will generate the index in the folder where the `.dat` file is located. The index is suffixed with `.ring` or `.cring` according to the second argument.

4. We are ready to run the code! We should have another executable file called `query-index`, then we should run:

```Bash
./query-index <absoulute-path-to-the-index-file> <absolute-path-to-the-query-file>
```

Here we need to give the path of the the file that contains all the queries. A folder called `Queries` contains two files of queries. We have to give the path of one of the files within it:

- If we selected the file `wikidata-filtered-enumerated.dat` we have to give the absolute path of the file called `Queries-wikidata-benchmark.txt`.
- If we selected the file `wikidata-enumerated.dat` we have to select the absolute path of the file called `Queries-bgps-limit1000.txt`.

Now we are finished! After running this step we will execute the queries. In console we should see the number of the query, the number of results and the time taken by each one of the queries.


---

At the moment, we can find the rest of the complementary material at [this webpage](http://compact-leapfrog.tk/). Note that we will find instructions to run the code there, and although the instructions are different from the ones in this repository, they should work too.


Running the benchmark of paper:

1. Go to the main directory of the project
2. modify `run_automated_test_wikidata_filtered_1000_600.sh` to point to the path where your ring index is located, for example:

python automated_test.py -i [PATH_TO_INDEX].[ring|cring] -q Queries/Queries-wikidata-benchmark.txt --OutputFolder experiment_output

3. Run the bash script: ./run_automated_test_wikidata_filtered_1000_600.sh
4. Confirm that plots/matplotlib/ containts *.txt files.
5. Plotting the runtime benchmark:
    5.1 go to plots/ and run the following python script:
    5.2 `python boxplots_by_type.py`
6. Plotting the bytes per triple vs. median tradeoff:
    6.1 go to plots/tradeoff/ and run the following python script:
    6.2 `python tradeoff_by_query_type.py`