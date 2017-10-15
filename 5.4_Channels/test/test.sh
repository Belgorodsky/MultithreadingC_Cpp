#!/bin/bash
who | sort | uniq -c | sort -nk1
