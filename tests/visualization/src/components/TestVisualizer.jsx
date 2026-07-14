import { useMemo, useState } from 'react';
import { parseTestResult } from '../utils/testParser.js';

const sample = '{"generator":"game_rng","samples":32768,"buckets":[2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048,2048],"one_bits":[]}';

export default function TestVisualizer() {
  const [source, setSource] = useState(sample);
  const parsed = useMemo(() => {
    try {
      return { value: parseTestResult(source), error: null };
    } catch (error) {
      return { value: null, error: error.message };
    }
  }, [source]);

  const maximum = parsed.value ? Math.max(...parsed.value.buckets, 1) : 1;
  return (
    <main style={{ fontFamily: 'system-ui', maxWidth: 900, margin: '2rem auto' }}>
      <h1>classical_rng distribution viewer</h1>
      <p>Paste JSON printed by <code>test_game_rng</code> or <code>test_crypto_rng</code>.</p>
      <textarea
        value={source}
        onChange={(event) => setSource(event.target.value)}
        rows={8}
        style={{ width: '100%', fontFamily: 'monospace' }}
      />
      {parsed.error && <p role="alert">{parsed.error}</p>}
      {parsed.value && (
        <section>
          <h2>{parsed.value.generator}: {parsed.value.samples} samples</h2>
          <div style={{ display: 'flex', alignItems: 'end', height: 260, gap: 8 }}>
            {parsed.value.buckets.map((count, index) => (
              <div key={index} style={{ flex: 1, textAlign: 'center' }}>
                <div
                  title={`${index}: ${count}`}
                  style={{
                    height: `${(count / maximum) * 220}px`,
                    background: '#315c8c',
                  }}
                />
                <small>{index}</small>
              </div>
            ))}
          </div>
          <p>A flat chart is a diagnostic, not proof of unpredictability.</p>
        </section>
      )}
    </main>
  );
}
