import os

def generate(filename, flags, dict):
    if os.path.exists(filename + ".exe"): return
    fp = open(f'{filename}.cpp', 'w')

    for flag in flags:
        fp.write(f'#define {flag}\n')
    fp.write('#include "core.h"\n\nvoid init()\n{\n')
    for k, v in dict.items():
        if k.startswith('_'): continue
        if '.' in k:
            k = k.split('.')
            fp.write(f'    ((random2 *)::cfg.{k[0]})->{k[1]} = {v};\n')
        else:
            fp.write(f'    ::cfg.{k} = {v};\n')
    fp.write(f'    ::cfg.savefile = "{filename}.log";\n')
    fp.write('}\n')
    fp.close()

    os.system(f'g++ {filename}.cpp -o {filename}.exe -O2')
    #exit(0)
    os.remove(f'{filename}.cpp')

def gen(ecp, pattern):
    '''
    generate(f'clwl_pattern{pattern["_id"]}_ecp{ecp}', ['column_wear_leveling', 'start_gap', f'ecp{ecp}'], {
        'row_rand.high': 32,
        'row_rand.low': 32,
        'row_rand.times': '1',
        'row_rand.percent': 0.5,
        'swap_endurance': '0.002',
        'mean': '100000',
        'cov': 0.3
    } | pattern)
    generate(f'enhanced_ilf_pattern{pattern["_id"]}_ecp{ecp}', ['ilf', 'start_gap', f'ecp{ecp}'], {
        'row_rand.high': 32,
        'row_rand.low': 32,
        'row_rand.times': '1',
        'row_rand.percent': 0.5,
        'swap_endurance': '0.002',
        'mean': '100000',
        'cov': 0.3
    } | pattern)
    generate(f'byte_rotation_pattern{pattern["_id"]}_ecp{ecp}', ['byte_rotation', 'start_gap', f'ecp{ecp}'], {
        'row_rand.high': 32,
        'row_rand.low': 32,
        'row_rand.times': '1',
        'row_rand.percent': 0.5,
        'swap_endurance': '0.002',
        'mean': '100000',
        'cov': 0.3
    } | pattern)
    for i in [1, 2, 4, 8, 16, 32, 64, 128, 256, 512]:
        generate(f'hwl_pattern{pattern["_id"]}_ecp{ecp}_group{i}', ['horizontal_wear_leveling', 'start_gap', f'ecp{ecp}'], {
            'row_rand.high': 32,
            'row_rand.low': 32,
            'row_rand.times': '1',
            'row_rand.percent': 0.5,
            'swap_endurance': '0.002',
            'mean': '100000',
            'cov': 0.3,
            'group_bits': i
        } | pattern)'''
    '''
    generate(f'hwlp_pattern{pattern["_id"]}_ecp{ecp}', ['hwl_plus', 'start_gap', f'ecp{ecp}'], {
        'row_rand.high': 32,
        'row_rand.low': 32,
        'row_rand.times': '1',
        'row_rand.percent': 0.5,
        'swap_endurance': '0.002',
        'mean': '100000',
        'cov': 0.3
    } | pattern)'''
    
'''
pattern1 = {
    'col_rand.high': 20,
    'col_rand.low': 14,
    'col_rand.times': 15,
    'col_rand.percent': 1,
    '_id': 1
}

pattern2 = {
    'col_rand': 'new lsbrand()',
    '_id': 2
}

pattern3 = {
    'col_rand.high': '256', 
    'col_rand.low': '256',
    'col_rand.times': '1',
    'col_rand.percent': '0.5',
    '_id': 0
}

pattern4 = {
    'col_rand.high': '64',
    'col_rand.low': '448',
    'col_rand.times': '1',
    'col_rand.percent': '1',
    '_id':4
}

generate(f'none_ecp{1}_cov{0.3}', [f'ecp{1}'], {
    'row_rand.high': 10,
    'row_rand.low': 54,
    'row_rand.times': '1',
    'row_rand.percent': 1,
    'col_rand.high': '256', 
    'col_rand.low': '256',
    'col_rand.times': '1',
    'col_rand.percent': '0.5',
    'mean': '100000',
    'cov': 0.3
})

for ecp in [0, 1, 2, 3]:
    for cov in [0.15, 0.2, 0.25, 0.3, 0.25, 0.35]:
        pattern = pattern3
        generate(f'uniform_only_ecp{ecp}_cov{cov}', [f'ecp{ecp}', f'uniform'], {
            'row_rand.high': 10,
            'row_rand.low': 54,
            'row_rand.times': '1',
            'row_rand.percent': 1,
            'col_rand.high': '256', 
            'col_rand.low': '256',
            'col_rand.times': '1',
            'col_rand.percent': '0.5',
            'mean': '100000',
            'cov': cov
        })
        generate(f'none_ecp{ecp}_cov{cov}', [f'ecp{ecp}'], {
            'row_rand.high': 10,
            'row_rand.low': 54,
            'row_rand.times': '1',
            'row_rand.percent': 1,
            'col_rand.high': '256', 
            'col_rand.low': '256',
            'col_rand.times': '1',
            'col_rand.percent': '0.5',
            'mean': '100000',
            'cov': cov
        })
    
for ecp in [0, 1, 2, 3]:
    for cov in [0.05, 0.15, 0.2, 0.25, 0.3, 0.25, 0.35]:
        pattern = pattern3
        for i in [1,2,3,4]:
            generate(f'cairs{i}_pattern{pattern["_id"]}_ecp{ecp}_cov{cov}', ['cai', f'ecp{ecp}', 'row_sparing'], {
                'row_rand.high': 10,
                'row_rand.low': 54,
                'row_rand.times': 1,
                'row_rand.percent': 1,
                'swap_endurance': '0.002',
                'mean': '100000',
                'cov': cov,
                'rs_count': i,
                'row_swap_interval': '100'
            } | pattern)
            generate(f'random{i}_pattern{pattern["_id"]}_ecp{ecp}_cov{cov}', ['random', f'ecp{ecp}', 'row_sparing'], {
                'row_rand.high': 10,
                'row_rand.low': 54,
                'row_rand.times': 1,
                'row_rand.percent': 1,
                'swap_endurance': '0.002',
                'mean': '100000',
                'cov': cov,
                'rs_count': i
            } | pattern)

        generate(f'cai_pattern{pattern["_id"]}_ecp{ecp}_cov{cov}', ['cai', f'ecp{ecp}'], {
            'row_rand.high': 10,
            'row_rand.low': 54,
            'row_rand.times': 1,
            'row_rand.percent': 1,
            'swap_endurance': '0.002',
            'mean': '100000',
            'cov': cov,
        } | pattern)
        generate(f'random_pattern{pattern["_id"]}_ecp{ecp}_cov{cov}', ['random', f'ecp{ecp}'], {
            'row_rand.high': 10,
            'row_rand.low': 54,
            'row_rand.times': 1,
            'row_rand.percent': 1,
            'swap_endurance': '0.002',
            'mean': '100000',
            'cov': cov,
        } | pattern)

def gen(gap, ecp, cov, percent, flags = [], prefix = ''):
    generate(f'{prefix}new_retrofit_{percent}_{gap}gap_ecp{ecp}_cov{cov}', ['retrofit', f'ecp{ecp}'] + flags, {
        'row_rand.high': int(64 * percent),
        'row_rand.low': 64 - int(64 * percent),
        'row_rand.times': '1',
        'row_rand.percent': 1 - percent,
        'col_rand.high': '256', 
        'col_rand.low': '256',
        'col_rand.times': '1',
        'col_rand.percent': '0.5',
        'swap_endurance': '0.002',
        'retrofit_times_m': '256',
        'retrofit_times_n': '25',
        'extra_row': gap,
        'mean': '100000',
        'cov': cov
    })
    generate(f'{prefix}new_retrofit_gps_{percent}_{gap}gap_ecp{ecp}_cov{cov}', ['retrofit', f'ecp{ecp}', f'retrofit_global_pointer'] + flags, {
        'row_rand.high': int(64 * percent),
        'row_rand.low': 64 - int(64 * percent),
        'row_rand.times': '1',
        'row_rand.percent': 1 - percent,
        'col_rand.high': '256', 
        'col_rand.low': '256',
        'col_rand.times': '1',
        'col_rand.percent': '0.5',
        'swap_endurance': '0.002',
        'retrofit_times_m': '256',
        'retrofit_times_n': '25',
        'extra_row': gap,
        'mean': '100000',
        'cov': cov
    })
    generate(f'{prefix}new_uniform_{percent}_{gap}gap_ecp{ecp}_cov{cov}', [f'ecp{ecp}', f'uniform', 'row_sparing'] + flags, {
        'row_rand.high': int(64 * percent),
        'row_rand.low': 64 - int(64 * percent),
        'row_rand.times': '1',
        'row_rand.percent': 1 - percent,
        'col_rand.high': '256', 
        'col_rand.low': '256',
        'col_rand.times': '1',
        'col_rand.percent': '0.5',
        'mean': '100000',
        'rs_count': gap,
        'cov': cov
    })
    
    generate(f'{prefix}new_start_gap_{percent}_{gap}gap_ecp{ecp}_cov{cov}', [f'ecp{ecp}', f'start_gap', f'row_sparing'] + flags, {
        'row_rand.high': int(64 * percent),
        'row_rand.low': 64 - int(64 * percent),
        'row_rand.times': '1',
        'row_rand.percent': 1 - percent,
        'col_rand.high': '256', 
        'col_rand.low': '256',
        'col_rand.times': '1',
        'col_rand.percent': '0.5',
        'swap_endurance': '0.002',
        'mean': '100000',
        'rs_count': gap - 1,
        'cov': cov
    })
    generate(f'{prefix}new_row_sparing_{percent}_{gap}gap_ecp{ecp}_cov{cov}', [f'ecp{ecp}', f'row_sparing'] + flags, {
        'row_rand.high': int(64 * percent),
        'row_rand.low': 64 - int(64 * percent),
        'row_rand.times': '1',
        'row_rand.percent': 1 - percent,
        'col_rand.high': '256', 
        'col_rand.low': '256',
        'col_rand.times': '1',
        'col_rand.percent': '0.5',
        'rs_count': gap,
        'mean': '100000',
        'cov': cov
    })
    generate(f'{prefix}new_cairs_{percent}_{gap}gap_ecp{ecp}_cov{cov}', [f'ecp{ecp}', 'cai', f'row_sparing'] + flags, {
        'row_rand.high': int(64 * percent),
        'row_rand.low': 64 - int(64 * percent),
        'row_rand.times': '1',
        'row_rand.percent': 1 - percent,
        'col_rand.high': '256', 
        'col_rand.low': '256',
        'col_rand.times': '1',
        'col_rand.percent': '0.5',
        'swap_endurance': '0.002',
        'rs_count': gap,
        'mean': '100000',
        'cov': cov,
    })
    generate(f'{prefix}new_random_{percent}_{gap}gap_ecp{ecp}_cov{cov}', [f'ecp{ecp}', 'random', f'row_sparing'] + flags, {
        'row_rand.high': int(64 * percent),
        'row_rand.low': 64 - int(64 * percent),
        'row_rand.times': '1',
        'row_rand.percent': 1 - percent,
        'col_rand.high': '256', 
        'col_rand.low': '256',
        'col_rand.times': '1',
        'col_rand.percent': '0.5',
        'swap_endurance': '0.002',
        'rs_count': gap,
        'mean': '100000',
        'cov': cov,
    })

for percent in [1/64, 4/64, 8/64, 12/64, 16/64]:
    gen(gap=8, ecp=1, cov=0.3, percent=percent)

for cov in [0.15, 0.2, 0.25, 0.3, 0.35]:
    for gap in [8]:
        gen(gap=gap, ecp=1, cov=cov, percent = 10/64)

for ecp in [0, 1, 2, 3]:
    gen(8, ecp, 0.3, 10/64)
    
gen(4, 0, 0.3, 10 / 64, ['ecc'], 'ecc_')
gen(4, 1, 0.3, 10 / 64, ['ecc'], 'ecc_')
for pattern in [pattern1, pattern2, pattern4]:
    for ecp in [0, 1, 2, 3]:
        for gap in [8]:
            generate(f'hwl_pattern{pattern["_id"]}_gap{gap}_ecp{ecp}_group{i}', ['horizontal_wear_leveling', 'start_gap', 'row_sparing', f'ecp{ecp}'], {
                        'row_rand.high': 10,
                        'row_rand.low': 54,
                        'row_rand.times': 1,
                        'row_rand.percent': 1,
                        'swap_endurance': '0.002',
                        'mean': '100000',
                        'rs_count': gap - 1,
                        'cov': 0.3,
                    } | pattern)
'''
def gen(gap, ecp, cov, percent):
    generate(f'retrofit_{percent}_{gap}gap_ecp{ecp}_cov{cov}', ['retrofit', f'ecp{ecp}'], {
        'row_rand.high': int(64 * percent),
        'row_rand.low': 64 - int(64 * percent),
        'row_rand.times': '1',
        'row_rand.percent': 1 - percent,
        'col_rand.high': '256', 
        'col_rand.low': '256',
        'col_rand.times': '1',
        'col_rand.percent': '0.5',
        'swap_endurance': '0.002',
        'retrofit_times_m': '256',
        'retrofit_times_n': '25',
        'extra_row': gap,
        'mean': '100000',
        'cov': cov
    })
    generate(f'retrofit_gps_{percent}_{gap}gap_ecp{ecp}_cov{cov}', ['retrofit', f'ecp{ecp}', f'retrofit_global_pointer'], {
        'row_rand.high': int(64 * percent),
        'row_rand.low': 64 - int(64 * percent),
        'row_rand.times': '1',
        'row_rand.percent': 1 - percent,
        'col_rand.high': '256', 
        'col_rand.low': '256',
        'col_rand.times': '1',
        'col_rand.percent': '0.5',
        'swap_endurance': '0.002',
        'retrofit_times_m': '256',
        'retrofit_times_n': '25',
        'extra_row': gap,
        'mean': '100000',
        'cov': cov
    })
    generate(f'uniform_{percent}_{gap}gap_ecp{ecp}_cov{cov}', [f'ecp{ecp}', f'uniform', 'row_sparing'], {
        'row_rand.high': int(64 * percent),
        'row_rand.low': 64 - int(64 * percent),
        'row_rand.times': '1',
        'row_rand.percent': 1 - percent,
        'col_rand.high': '256', 
        'col_rand.low': '256',
        'col_rand.times': '1',
        'col_rand.percent': '0.5',
        'mean': '100000',
        'rs_count': gap,
        'cov': cov
    })
    generate(f'uniform_gps_{percent}_{gap}gap_ecp{ecp}_cov{cov}', [f'ecp{ecp}', f'uniform', 'row_sparing', 'rs_global_pointer'], {
        'row_rand.high': int(64 * percent),
        'row_rand.low': 64 - int(64 * percent),
        'row_rand.times': '1',
        'row_rand.percent': 1 - percent,
        'col_rand.high': '256', 
        'col_rand.low': '256',
        'col_rand.times': '1',
        'col_rand.percent': '0.5',
        'mean': '100000',
        'rs_count': gap,
        'cov': cov
    })
    generate(f'row_sparing_{percent}_{gap}gap_ecp{ecp}_cov{cov}', [f'ecp{ecp}', f'row_sparing'], {
        'row_rand.high': int(64 * percent),
        'row_rand.low': 64 - int(64 * percent),
        'row_rand.times': '1',
        'row_rand.percent': 1 - percent,
        'col_rand.high': '256', 
        'col_rand.low': '256',
        'col_rand.times': '1',
        'col_rand.percent': '0.5',
        'rs_count': gap,
        'mean': '100000',
        'cov': cov
    })
    generate(f'row_sparing_gps_{percent}_{gap}gap_ecp{ecp}_cov{cov}', [f'ecp{ecp}', f'row_sparing', f'rs_global_pointer'], {
        'row_rand.high': int(64 * percent),
        'row_rand.low': 64 - int(64 * percent),
        'row_rand.times': '1',
        'row_rand.percent': 1 - percent,
        'col_rand.high': '256', 
        'col_rand.low': '256',
        'col_rand.times': '1',
        'col_rand.percent': '0.5',
        'rs_count': gap,
        'mean': '100000',
        'cov': cov
    })
    generate(f'start_gap_row_sparing_{percent}_{gap}gap_ecp{ecp}_cov{cov}', [f'ecp{ecp}', f'row_sparing', f'start_gap'], {
        'row_rand.high': int(64 * percent),
        'row_rand.low': 64 - int(64 * percent),
        'row_rand.times': '1',
        'row_rand.percent': 1 - percent,
        'col_rand.high': '256', 
        'col_rand.low': '256',
        'col_rand.times': '1',
        'col_rand.percent': '0.5',
        'swap_endurance': '0.002',
        'rs_count': gap - 1,
        'mean': '100000',
        'cov': cov
    })
    generate(f'start_gap_row_sparing_gps_{percent}_{gap}gap_ecp{ecp}_cov{cov}', [f'ecp{ecp}', f'row_sparing', f'start_gap', f'rs_global_pointer'], {
        'row_rand.high': int(64 * percent),
        'row_rand.low': 64 - int(64 * percent),
        'row_rand.times': '1',
        'row_rand.percent': 1 - percent,
        'col_rand.high': '256', 
        'col_rand.low': '256',
        'col_rand.times': '1',
        'col_rand.percent': '0.5',
        'swap_endurance': '0.002',
        'rs_count': gap - 1,
        'mean': '100000',
        'cov': cov
    })

gen(gap=4, ecp=1, cov=0.3, percent=10/64)
exit(0)
for gap in [1, 2, 4, 8]:
    for ecp in [0, 1, 2, 3]:
        gen(gap, ecp, cov=0.3, percent = 10/64)

for cov in [0.15, 0.2, 0.25, 0.3, 0.35]:
    for gap in [1, 2, 8]:
        gen(gap=gap, ecp=1, cov=cov, percent = 10/64)

for percent in [5/64, 10/64, 15/64, 20/64, 25/64, 30/64]:
    for gap in [1, 2, 4, 8]:
        gen(gap=gap, ecp=1, cov=0.3, percent=percent)

for percent in [1/64, 4/64, 8/64, 16/64, 32/64]:
    gen(gap=1, ecp=1, cov=0.3, percent=percent)

# dsp
# column_wear_leveling
# ilf
# byte_rotation
# ecp0 ecp1 ecp2 ecp3 ecp4



for ecp in [0, 1, 2, 3]:
    for gap in [2, 4,8]:
        for fault in [0]:
            '''
            generate(f'cairs_startfault{fault}_gap{gap}_ecp{ecp}', ['cai', 'row_sparing', f'ecp{ecp}'], {
                'row_rand.high': 10,
                'row_rand.low': 54,
                'row_rand.times': 1,
                'row_rand.percent': 1,
                'swap_endurance': '0.002',
                'mean': '100000',
                'rs_count': gap - 1,
                'cov': 0.3,
                'start_fault': fault
            } | pattern3)

            generate(f'retrofitgps_startfault{fault}_gap{gap}_ecp{ecp}', ['retrofit', 'retrofit_global_pointer', f'ecp{ecp}'], {
                'row_rand.high': 10,
                'row_rand.low': 54,
                'row_rand.times': 1,
                'row_rand.percent': 1,
                'swap_endurance': '0.002',
                'mean': '100000',
                'retrofit_times_m': '256',
                'retrofit_times_n': '25',
                'extra_row': gap,
                'cov': 0.3,
                'start_fault': fault
            } | pattern3)
'''            
            for paatern in [pattern1, pattern2, pattern4]:
                generate(f'byte_rotation+start_gap_pattern{paatern["_id"]}_startfault{fault}_gap{gap}_ecp{ecp}', ['start_gap', 'byte_rotation', f'ecp{ecp}', 'row_sparing'], {
    'row_rand.high': 10,
    'row_rand.low': 54,
    'row_rand.times': 1,
    'row_rand.percent': 1,
                    'swap_endurance': '0.002',
                    'mean': '100000',
                    'rs_count': gap - 1,
                    'cov': 0.3,
                    'start_fault': fault
                } | paatern)

                generate(f'enhanced_ilf+start_gap_pattern{paatern["_id"]}_startfault{fault}_gap{gap}_ecp{ecp}', ['start_gap', 'ilf', f'ecp{ecp}', 'row_sparing'], {
    'row_rand.high': 10,
    'row_rand.low': 54,
    'row_rand.times': 1,
    'row_rand.percent': 1,
                    'swap_endurance': '0.002',
                    'mean': '100000',
                    'rs_count': gap - 1,
                    'cov': 0.3,
                    'start_fault': fault
                } | paatern)

                generate(f'hwl2+start_gap_pattern{paatern["_id"]}_startfault{fault}_gap{gap}_ecp{ecp}', ['start_gap', 'horizontal_wear_leveling', f'ecp{ecp}', 'row_sparing'], {
    'row_rand.high': 10,
    'row_rand.low': 54,
    'row_rand.times': 1,
    'row_rand.percent': 1,
                    'swap_endurance': '0.002',
                    'mean': '100000',
                    'rs_count': gap - 1,
                    'cov': 0.3,
                    'start_fault': fault
                } | paatern)

                generate(f'column_wear_leveling2+retrofit_pattern{paatern["_id"]}_startfault{fault}_gap{gap}_ecp{ecp}', ['retrofit', 'hwl_plus', f'ecp{ecp}'], {
    'row_rand.high': 10,
    'row_rand.low': 54,
    'row_rand.times': 1,
    'row_rand.percent': 1,
                    'retrofit_times_m': '100',
                    'retrofit_times_n': '10',
                    'swap_endurance': '0.002',
                    'mean': '100000',
                    'extra_row': gap,
                    'cov': 0.3,
                    'start_fault': fault
                } | paatern)

                generate(f'column_wear_leveling2+retrofit_gps_pattern{paatern["_id"]}_startfault{fault}_gap{gap}_ecp{ecp}', ['retrofit', 'retrofit_global_pointer', 'hwl_plus', f'ecp{ecp}'], {
    'row_rand.high': 10,
    'row_rand.low': 54,
    'row_rand.times': 1,
    'row_rand.percent': 1,
                    'retrofit_times_m': '100',
                    'retrofit_times_n': '10',
                    'swap_endurance': '0.002',
                    'mean': '100000',
                    'extra_row': gap,
                    'cov': 0.3,
                    'start_fault': fault
                } | paatern)

'''
generate(f'test_pointer_uniform', ['uniform', 'row_sparing', 'ecp3'], {
    'row_rand.high': 10,
    'row_rand.low': 54,
    'row_rand.times': 1,
    'row_rand.percent': 1,
    'mean': '100000',
    'extra_row': 2,
    'cov': 0.3,
} | pattern3)

generate(f'test_pointer_startgap', ['start_gap', 'row_sparing', 'ecp3'], {
    'row_rand.high': 10,
    'row_rand.low': 54,
    'row_rand.times': 1,
    'row_rand.percent': 1,
    'mean': '100000',
    'extra_row': 1,
    'cov': 0.3,
    'swap_endurance': '0.002',
} | pattern3)

generate(f'test_pointer_retrofit', ['retrofit', 'ecp3'], {
    'row_rand.high': 10,
    'row_rand.low': 54,
    'row_rand.times': 1,
    'row_rand.percent': 1,
    'retrofit_times_m': '100',
    'retrofit_times_n': '10',
    'mean': '100000',
    'extra_row': 2,
    'swap_endurance': '0.002',
} | pattern3)

generate(f'test_pointer_retrofit4', ['retrofit', 'ecp3'], {
    'row_rand.high': 10,
    'row_rand.low': 54,
    'row_rand.times': 1,
    'row_rand.percent': 1,
    'retrofit_times_m': '100',
    'retrofit_times_n': '10',
    'mean': '100000',
    'extra_row': 4,
    'swap_endurance': '0.002',
} | pattern3)

generate(f'test_pointer_retrofitgps', ['retrofit', 'retrofit_global_pointer', 'ecp3'], {
    'row_rand.high': 10,
    'row_rand.low': 54,
    'row_rand.times': 1,
    'row_rand.percent': 1,
    'retrofit_times_m': '100',
    'retrofit_times_n': '10',
    'mean': '100000',
    'extra_row': 2,
    'swap_endurance': '0.002',
} | pattern3)

generate(f'test_pointer_retrofitgps4', ['retrofit', 'retrofit_global_pointer', 'ecp3'], {
    'row_rand.high': 10,
    'row_rand.low': 54,
    'row_rand.times': 1,
    'row_rand.percent': 1,
    'retrofit_times_m': '100',
    'retrofit_times_n': '10',
    'mean': '100000',
    'extra_row': 4,
    'swap_endurance': '0.002',
} | pattern3)

generate(f'test_pointer_retrofitgps8', ['retrofit', 'retrofit_global_pointer', 'ecp3'], {
    'row_rand.high': 10,
    'row_rand.low': 54,
    'row_rand.times': 1,
    'row_rand.percent': 1,
    'retrofit_times_m': '100',
    'retrofit_times_n': '10',
    'mean': '100000',
    'extra_row': 8,
    'swap_endurance': '0.002',
} | pattern3)
'''