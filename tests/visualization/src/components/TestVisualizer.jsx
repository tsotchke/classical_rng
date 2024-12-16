import React, { useState, useEffect } from 'react';
import { LineChart, XAxis, YAxis, Tooltip, Legend, Line, BarChart, Bar } from 'recharts';
import { readTestResults } from '../testParser';

const TestVisualizer = () => {
  const [selectedRng, setSelectedRng] = useState('game');
  const [testData, setTestData] = useState(null);
  const [error, setError] = useState(null);

  useEffect(() => {
    try {
      const testResults = readTestResults();
      setTestData(testResults);
    } catch (err) {
      setError("Failed to load test results");
      console.error(err);
    }
  }, []);

  if (error) {
    return <div className="p-4 text-red-500">{error}</div>;
  }

  if (!testData) {
    return (
      <div className="p-8">
        <div className="flex justify-between items-center mb-8">
          <h1 className="text-3xl font-bold">RNG Test Results</h1>
          <select 
            value={selectedRng}
            onChange={(e) => setSelectedRng(e.target.value)}
            className="border p-2 rounded"
          >
            <option value="game">Game RNG</option>
            <option value="crypto">Crypto RNG</option>
          </select>
        </div>

        <div className="grid grid-cols-1 lg:grid-cols-2 gap-8">
          {/* Performance Metrics */}
          <div className="bg-white p-6 rounded shadow">
            <h2 className="text-xl font-semibold mb-4">Performance Metrics</h2>
            <div className="grid grid-cols-2 gap-4">
              <div className="p-4 bg-gray-50 rounded animate-pulse"></div>
              <div className="p-4 bg-gray-50 rounded animate-pulse"></div>
              <div className="p-4 bg-gray-50 rounded animate-pulse"></div>
              <div className="p-4 bg-gray-50 rounded animate-pulse"></div>
            </div>
          </div>

          {/* Transition Matrix */}
          <div className="bg-white p-6 rounded shadow">
            <h2 className="text-xl font-semibold mb-4">Transition Matrix</h2>
            <div className="grid grid-cols-2 gap-2">
              {Array.from({ length: 4 }).map((_, i) => (
                <div key={i} className="p-4 bg-gray-50 rounded animate-pulse"></div>
              ))}
            </div>
          </div>

          {/* Value Distribution */}
          <div className="bg-white p-6 rounded shadow">
            <h2 className="text-xl font-semibold mb-4">Value Distribution</h2>
            <div className="h-[300px] bg-gray-50 rounded animate-pulse"></div>
          </div>

          {/* Bit Distribution */}
          <div className="bg-white p-6 rounded shadow">
            <h2 className="text-xl font-semibold mb-4">Bit Distribution</h2>
            <div className="h-[300px] bg-gray-50 rounded animate-pulse"></div>
          </div>
        </div>
      </div>
    );
  }

  const currentData = testData[selectedRng];
  
  const distributionData = Object.keys(currentData.distribution).map((bucket) => ({
    bucket: parseInt(bucket),
    count: currentData.distribution[bucket]
  }));

  const bitData = Object.keys(currentData.bit_counts).map((bit) => ({
    bit: parseInt(bit), 
    count: currentData.bit_counts[bit]
  }));

  return (
    <div className="p-8">
      <div className="flex justify-between items-center mb-8">
        <h1 className="text-3xl font-bold">RNG Test Results</h1>
        <select 
          value={selectedRng}
          onChange={(e) => setSelectedRng(e.target.value)}
          className="border p-2 rounded"
        >
          <option value="game">Game RNG</option>
          <option value="crypto">Crypto RNG</option>
        </select>
      </div>

      <div className="grid grid-cols-1 lg:grid-cols-2 gap-8">
        {/* Performance Metrics */}
        <div className="bg-white p-6 rounded shadow">
          <h2 className="text-xl font-semibold mb-4">Performance Metrics</h2>
          <div className="grid grid-cols-2 gap-4">
            <div className="p-4 bg-gray-50 rounded">
              <div className="text-sm text-gray-600">Chi Square</div>
              <div className="text-lg font-medium">{currentData.metrics.chi_square.toFixed(3)}</div>
            </div>
            <div className="p-4 bg-gray-50 rounded">
              <div className="text-sm text-gray-600">Bit Entropy</div>
              <div className="text-lg font-medium">{currentData.metrics.bit_entropy.toFixed(3)}</div>
            </div>
            <div className="p-4 bg-gray-50 rounded">
              <div className="text-sm text-gray-600">Generation Time</div>
              <div className="text-lg font-medium">{currentData.metrics.generation_time.toFixed(3)}s</div>
            </div>
            <div className="p-4 bg-gray-50 rounded">
              <div className="text-sm text-gray-600">Numbers/Second</div>
              <div className="text-lg font-medium">{currentData.metrics.numbers_per_second.toLocaleString()}</div>
            </div>
          </div>
        </div>

        {/* Transition Matrix */}
        <div className="bg-white p-6 rounded shadow">
          <h2 className="text-xl font-semibold mb-4">Transition Matrix</h2>
          <div className="grid grid-cols-2 gap-2">
            {Object.keys(currentData.transition_matrix).map((i) => 
              Object.keys(currentData.transition_matrix[i]).map((j) => (
                <div key={`${i}-${j}`} className="p-4 bg-gray-50 rounded text-center">
                  <div className="text-sm text-gray-600">{i}{' => '}{j}</div>
                  <div className="text-lg font-medium">{currentData.transition_matrix[i][j].toLocaleString()}</div>
                </div>
              ))
            )}
          </div>
        </div>

        {/* Value Distribution */}
        <div className="bg-white p-6 rounded shadow">
          <h2 className="text-xl font-semibold mb-4">Value Distribution</h2>
          <LineChart width={600} height={300} data={distributionData}>
            <XAxis dataKey="bucket" />
            <YAxis />
            <Tooltip />
            <Legend />
            <Line type="monotone" dataKey="count" stroke="#4f46e5" strokeWidth={2} dot={false} />
          </LineChart>
        </div>

        {/* Bit Distribution */}
        <div className="bg-white p-6 rounded shadow">
          <h2 className="text-xl font-semibold mb-4">Bit Distribution</h2>
          <BarChart width={600} height={300} data={bitData}>
            <XAxis dataKey="bit" />
            <YAxis />
            <Tooltip />
            <Legend />
            <Bar dataKey="count" fill="#4f46e5" />
          </BarChart>
        </div>
      </div>
    </div>
  );
};

export default TestVisualizer;