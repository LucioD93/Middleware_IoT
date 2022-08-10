#!/usr/bin/env python3

import json
import matplotlib.pyplot as plt

import pandas as pd
import matplotlib.pyplot as plt
from itertools import groupby


def add_line(ax, xpos, ypos):
    line = plt.Line2D(
        [xpos, xpos],
        [ypos + .1, ypos],
        transform=ax.transAxes,
        color='gray'
    )
    line.set_clip_on(False)
    ax.add_line(line)

def label_len(my_index,level):
    labels = my_index.get_level_values(level)
    return [(k, sum(1 for i in g)) for k,g in groupby(labels)]


def label_group_bar_table(ax, df):
    ypos = -.1
    scale = 1./df.index.size
    breakpoint()
    for level in range(df.index.nlevels)[::-1]:
        pos = 0
        for label, rpos in label_len(df.index,level):
            lxpos = (pos + .5 * rpos)*scale
            ax.text(lxpos, ypos, label, ha='center', transform=ax.transAxes)
            add_line(ax, pos*scale, ypos)
            pos += rpos
        add_line(ax, pos*scale , ypos)
        ypos -= .1


def get_values(path):
    data = []

    number_of_requests = ['20', '100', '200', '500', '1000', '2000']
    # number_of_requests = [2000]
    request_types = [1, 2, 3, 4, 5, 6, 7]

    for type in request_types:
        for number in number_of_requests:
            with open(f"{path}/ny_client_logs/log-client-{number}-{type}.txt", "r") as log:
                total_time_line = log.readline()
                total_time_value = float(total_time_line.split(": ")[1].strip())
                data.append([str(number), 'total', str(type), total_time_value])
                min_time_line = log.readline()
                min_time_value = float(min_time_line.split(": ")[1].strip())
                data.append([str(number), 'min', str(type), min_time_value])
                max_time_line = log.readline()
                max_time_value = float(max_time_line.split(": ")[1].strip())
                data.append([str(number), 'max', str(type), max_time_value])
                average_time_line = log.readline()
                average_time_value = float(average_time_line.split(": ")[1].strip())
                data.append([str(number), 'promedio', str(type), average_time_value])
    
    return data


if __name__ == "__main__":
    print("Plotting")
    data = get_values("logs")
    df = pd.DataFrame(data, columns = ['cantidad','medicion', 'tipo','tiempo']) 
    df = df.set_index(['cantidad','medicion','tipo'])['tiempo'].unstack()

    ax = df.plot(kind='bar')
    #Below 2 lines remove default labels
    ax.set_xticklabels('')
    ax.set_xlabel('')
    label_group_bar_table(ax, df)

    # you may need these lines, if not working interactive
    # plt.tight_layout()
    plt.show()



# data = [
#     ['20','min','1',24457],
#     ['20','min','2',24480],
#     ['20','max','1',222867],
#     ['20','promedio','1',15],
#     ['20','total','1',83848],
#     ['100','min','1',169877],
#     ['100','max','1',10692],
#     ['100','promedio','1',2631],
#     ['100','total','1',235853],
#     ['200','min','1',169877],
#     ['200','max','1',10692],
#     ['200','promedio','1',2631],
#     ['200','total','1',235853],
# ]