export function readTestResults() {
  // Retrieve the 'game_rng_results' and 'crypto_rng_results' data from localStorage
  const gameRngData = JSON.parse(localStorage.getItem('game_rng_results') || '{}');
  const cryptoRngData = JSON.parse(localStorage.getItem('crypto_rng_results') || '{}');

  // Return an object containing the test results for both RNGs
  return {
    game_rng: gameRngData,
    crypto_rng: cryptoRngData
  };
}