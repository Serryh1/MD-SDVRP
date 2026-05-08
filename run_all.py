import os
import sys
import subprocess
import concurrent.futures
from pathlib import Path
from datetime import datetime

def redirect_print_to_file(log_file_path):

    log_dir = os.path.dirname(log_file_path)
    if log_dir: 
        os.makedirs(log_dir, exist_ok=True)

    log_file = open(log_file_path, 'wb', buffering=0)
  
    class RealTimeStdout:
        def write(self, data):
            if isinstance(data, str):
                log_file.write(data.encode('utf-8'))  
            else:
                log_file.write(data)  
        
        def flush(self):
            log_file.flush()
        
        def close(self):
            log_file.close()
    
    sys.stdout = RealTimeStdout()
    return log_file

def run_cpp_test(input_file, output_file, p_value, cpp_executable):
    try:
        os.makedirs(os.path.dirname(output_file), exist_ok=True)
        
        command = [cpp_executable, input_file, output_file, str(p_value)]
        
        result = subprocess.run(command, capture_output=True, text=True, check=True)
        
        return {
            'status': 'success',
            'input_file': input_file,
            'p_value': p_value,
            'stdout': result.stdout
        }
        
    except subprocess.CalledProcessError as e:
        return {
            'status': 'error',
            'input_file': input_file,
            'p_value': p_value,
            'stderr': e.stderr
        }

def main():
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    log_file_path = f"./test_logs/print_output_{timestamp}.log"
    
    log_file = redirect_print_to_file(log_file_path)
    
    try:
        cpp_executable = "./build/MDSDVRP_1"  
        
        # input   output
        folder_pairs = [
            ("./dataset/P_set", "./Result_with_rounds/P_set"),
            ("./dataset/SD_set/k=2", "./Result_with_rounds/k=2"),
            ("./dataset/SD_set/k=4", "./Result_with_rounds/k=4"),
            ("./dataset/SD_set/k=6", "./Result_with_rounds/k=6"),
            ("./dataset/G_set", "./Result/G_set_with_rounds")
        ]
        
        p_values = ["GW", "Match"]  
        
        tasks = []
        
        for input_folder, output_folder in folder_pairs:
            input_path = Path(input_folder)
            
            # 
            file_patterns = [
                "p*.txt",
                "SD*.txt",
                "g*.txt"
            ]
            
            test_files = []
            for pattern in file_patterns:
                test_files.extend(input_path.glob(pattern))
            
            test_files = list(set(test_files))
            
            for test_file in test_files:
                input_file = str(test_file)
                
                for p_value in p_values:
                    base_name = test_file.stem
                    output_file = f"{output_folder}/{base_name}_p{p_value}.txt"
                    
                    tasks.append((input_file, output_file, p_value, cpp_executable))
        
        print(f"start test，total {len(tasks)} tasks")
        
        with concurrent.futures.ThreadPoolExecutor(20) as executor:

            futures = [executor.submit(run_cpp_test, *task) for task in tasks]
            

            for future in concurrent.futures.as_completed(futures):
                result = future.result()
                if result['status'] == 'success':
                    print(f"success: {result['input_file']}, p={result['p_value']}")
                    if result['stdout']:
                        print(f"stdout: {result['stdout'].strip()}")
                else:
                    print(f"run error: {result['input_file']}, p={result['p_value']}")
                    if result['stderr']:
                        print(f"std error: {result['stderr'].strip()}")
        
        print("all test success")
    
    finally:
        sys.stdout.close()  
        sys.stdout = sys.__stdout__  
        print(f"   {log_file_path}")

if __name__ == "__main__":
    main()