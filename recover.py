import os, subprocess

missing_files = ["config.c", "multi_samus.c", "physics_config.c", "sm_80.c", "sm_82.c", "sm_84.c", "sm_88.c", "sm_8b.c", "sm_9b.c", "sm_a0.c", "sm_a7.c", "sm_a9.c", "sm_ad.c", "sm_cpu_infra.c", "sm_dispatcher.c"]

for fname in missing_files:
    try:
        out = subprocess.check_output(f"nm src/{fname.replace('.c', '.o')}", shell=True).decode()
    except Exception as e:
        print("Failed nm for", fname)
        continue
    
    symbols = [line.split()[-1] for line in out.splitlines() if ' T ' in line or ' t ' in line]
    if not symbols:
        print("No text symbols for", fname)
        continue
    
    # Try finding the blob that contains the most symbols
    best_blob = None
    best_score = -1
    blobs = os.listdir("recovered")
    for blob_name in blobs:
        blob_path = os.path.join("recovered", blob_name)
        if not os.path.isfile(blob_path): continue
        try:
            with open(blob_path, 'r', encoding='utf-8') as f:
                content = f.read()
            score = sum(1 for s in symbols if s in content)
            if score > best_score and score > 0:
                best_score = score
                best_blob = blob_path
        except:
            pass
            
    if best_blob:
        print(f"Mapped {fname} to {best_blob} (score: {best_score}/{len(symbols)})")
        os.system(f"cp {best_blob} src/{fname}")
    else:
        print(f"Failed to find blob for {fname}")
